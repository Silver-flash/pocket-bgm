#!/usr/bin/env python3
"""Loopback-only audio upload, preview and verified firmware packaging."""
import argparse
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
from pathlib import Path
import secrets
import subprocess
import sys
import threading
from urllib.parse import parse_qs, urlsplit
import uuid
import webbrowser

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT/'local-audio'
TOKEN = secrets.token_urlsafe(32)
LOCK = threading.Lock()
STATE = dict(busy=False, status='默认曲库已就绪，可试听、删除或添加音频。', ready=False)
EXTENSIONS = {'.mp3','.wav','.m4a','.aac','.flac','.ogg','.opus','.aif','.aiff','.wma','.mp4'}
MAX_BYTES = 32 * 1024 * 1024

def defaults():
    return json.loads((ROOT/'assets/music/builtin/catalog.json').read_text())
def library():
    path=DATA/'library-v2.json'
    if not path.exists():
        legacy=DATA/'library.json'
        items=defaults()+(json.loads(legacy.read_text()) if legacy.exists() else [])
        save_library(items)
    return json.loads(path.read_text())
def save_library(items):
    temp=DATA/'library-v2.tmp'
    temp.write_text(json.dumps(items,ensure_ascii=False,indent=2))
    temp.replace(DATA/'library-v2.json')
def audio_path(item):
    return ROOT/item['path'] if item.get('builtin') else DATA/(item['id']+'.wav')
def build():
    try:
        files=library()
        selection=[dict(path=str(audio_path(f)),title=f['title'] if f.get('builtin') else f'我的音频 {i+1:02}',source=f['name']) for i,f in enumerate(files)]
        (DATA/'build-selection.json').write_text(json.dumps(selection,ensure_ascii=False))
        with (DATA/'build.log').open('w') as log:
            subprocess.run([sys.executable,str(ROOT/'tools/prepare_bgm.py'),
                            '--library',str(DATA/'build-selection.json')], cwd=ROOT,
                           stdout=log,stderr=log,check=True,timeout=180)
            subprocess.run(['bash',str(ROOT/'tools/build_bgm.sh')],cwd=ROOT,
                           stdout=log,stderr=log,check=True,timeout=1800)
        with LOCK: STATE.update(status='安装文件已生成并通过校验。可以下载了。',ready=True)
    except (subprocess.SubprocessError,OSError) as e:
        with LOCK: STATE.update(status='生成失败：请检查音频总时长与开发环境。详细记录保存在 local-audio/build.log。',ready=False)
    finally:
        with LOCK: STATE['busy']=False

class Handler(BaseHTTPRequestHandler):
    def log_message(self,*args): pass
    def json(self,data,code=200):
        body=json.dumps(data,ensure_ascii=False).encode()
        self.send_response(code);self.send_header('Content-Type','application/json; charset=utf-8')
        self.send_header('Cache-Control','no-store');self.send_header('Content-Length',str(len(body)));self.end_headers();self.wfile.write(body)
    def local_host(self):
        return self.headers.get('Host')==f'127.0.0.1:{self.server.server_port}'
    def send_file(self,path,mime,download=False):
        if not path.is_file(): return self.json({'error':'文件不存在'},404)
        self.send_response(200);self.send_header('Content-Type',mime)
        self.send_header('Content-Length',str(path.stat().st_size))
        if download:self.send_header('Content-Disposition','attachment; filename="Pocket-BGM-full.bin"')
        self.end_headers()
        with path.open('rb') as f:
            while chunk:=f.read(65536):self.wfile.write(chunk)
    def do_GET(self):
        if not self.local_host():return self.json({'error':'仅允许本机访问'},403)
        path=urlsplit(self.path).path
        if path=='/':return self.send_file(ROOT/'tools/bgm-studio/index.html','text/html; charset=utf-8')
        if path=='/api/state':
            with LOCK:return self.json(dict(STATE,files=library(),token=TOKEN))
        if path.startswith('/audio/'):
            ident=path.rsplit('/',1)[-1]
            if ident not in [f['id'] for f in library()]:return self.json({'error':'曲目不存在'},404)
            return self.send_file(audio_path(next(f for f in library() if f['id']==ident)),'audio/wav')
        if path=='/firmware' and STATE['ready']:
            return self.send_file(ROOT/'build/FoloToy-AI-Passport-full.bin','application/octet-stream',True)
        return self.json({'error':'未找到'},404)
    def do_POST(self):
        if not self.local_host() or self.headers.get('X-BGM-Token')!=TOKEN:
            return self.json({'error':'请刷新本地页面后重试'},403)
        try: length=int(self.headers.get('Content-Length','0'))
        except ValueError:return self.json({'error':'无效文件大小'},400)
        if length<=0 or length>MAX_BYTES:return self.json({'error':'单个文件需在 0–32 MB 之间'},400)
        parsed=urlsplit(self.path)
        with LOCK:
            if STATE['busy']:return self.json({'error':'正在生成，请稍候'},409)
            STATE['busy']=True
        try:
            if parsed.path=='/api/upload':
                items=library()
                if len(items)>=24:raise ValueError('最多 24 段，请先移除曲目')
                name=Path(parse_qs(parsed.query).get('name',[''])[0]).name
                ext=Path(name).suffix.lower()
                if ext not in EXTENSIONS:raise ValueError('请选择 MP3、WAV、M4A、AAC、FLAC、OGG 等音频文件')
                ident=uuid.uuid4().hex
                source=DATA/(ident+ext); output=DATA/(ident+'.preview.wav')
                self.connection.settimeout(60)
                source.write_bytes(self.rfile.read(length))
                try:
                    # Bound decoding to slightly more than the full pack capacity; reject oversized clips.
                    subprocess.run(['ffmpeg','-v','error','-nostdin','-i',str(source),'-t','641','-vn',
                                    '-ac','1','-ar','16000','-c:a','pcm_s16le',str(output)],
                                   stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,check=True,timeout=90)
                    duration=(output.stat().st_size-44)/32000
                    existing=sum(f['seconds'] for f in items)
                    if duration < .02:raise ValueError('音频为空')
                    if duration+existing>640:raise ValueError('曲库总时长超过 640 秒，请先删除或裁短曲目')
                    target=DATA/(ident+'.wav')
                    # Source WAV and destination may share the same name; preserve only normalized output.
                    source.unlink(missing_ok=True);output.replace(target)
                    items.append(dict(id=ident,name=name,seconds=round(duration,2)))
                    save_library(items)
                finally:
                    if source.suffix!='.wav' or not any(f['id']==ident for f in items):source.unlink(missing_ok=True)
                    output.unlink(missing_ok=True)
                STATE.update(status='音频已加入曲库，可试听并生成安装文件。',ready=False)
                return self.json({'ok':True})
            if parsed.path=='/api/delete':
                ident=json.loads(self.rfile.read(length)).get('id')
                items=library(); found=[f for f in items if f['id']==ident]
                if not found:raise ValueError('曲目不存在')
                save_library([f for f in items if f['id']!=ident])
                if not found[0].get('builtin'):(DATA/(ident+'.wav')).unlink(missing_ok=True)
                STATE.update(status='曲目已移除，请重新生成安装文件。',ready=False)
                return self.json({'ok':True})
            if parsed.path=='/api/restore':
                self.rfile.read(length)
                items=library();ids={f['id'] for f in items}
                missing=[f for f in defaults() if f['id'] not in ids]
                if len(items)+len(missing)>24 or sum(f['seconds'] for f in items+missing)>640:
                    raise ValueError('曲库空间不足，请先删除部分曲目')
                save_library(items+missing)
                STATE.update(status='默认曲目已恢复，请重新生成安装文件。',ready=False)
                return self.json({'ok':True})
            if parsed.path=='/api/build':
                self.rfile.read(length)
                STATE.update(status='正在转换音频、编译与校验安装文件，请保留此窗口。',ready=False)
                threading.Thread(target=build,daemon=True).start()
                return self.json({'ok':True})
            return self.json({'error':'未找到'},404)
        except ValueError as e:return self.json({'error':str(e)},400)
        except (subprocess.SubprocessError,OSError):return self.json({'error':'无法读取音频，请检查文件是否完整，或换成 MP3/WAV 后重试'},400)
        finally:
            if parsed.path!='/api/build':
                with LOCK:STATE['busy']=False

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--port',type=int,default=8765);parser.add_argument('--no-open',action='store_true');args=parser.parse_args()
    DATA.mkdir(exist_ok=True)
    server=ThreadingHTTPServer(('127.0.0.1',args.port),Handler)
    print(f'曲库工作台 http://127.0.0.1:{server.server_port}',flush=True)
    if not args.no_open:webbrowser.open(f'http://127.0.0.1:{server.server_port}')
    server.serve_forever()
if __name__=='__main__':main()
