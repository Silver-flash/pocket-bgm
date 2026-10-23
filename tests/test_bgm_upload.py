"""Real FFmpeg + HTTP integration test; all uploads stay in a temporary folder."""
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import threading
import unittest
from urllib.request import Request,urlopen
from urllib.error import HTTPError

root=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('studio',root/'tools/bgm_studio.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class UploadTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp=tempfile.TemporaryDirectory();m.DATA=Path(cls.tmp.name)
        cls.server=m.ThreadingHTTPServer(('127.0.0.1',0),m.Handler)
        cls.base=f'http://127.0.0.1:{cls.server.server_port}'
        threading.Thread(target=cls.server.serve_forever,daemon=True).start()
    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown();cls.server.server_close();cls.tmp.cleanup()
    def request(self,path,data=None,token=True):
        headers={'X-BGM-Token':m.TOKEN} if token else {}
        return urlopen(Request(self.base+path,data=data,headers=headers),timeout=30)
    def test_builtin_deletion_persists(self):
        items=m.library();builtins=[f for f in items if f.get('builtin')]
        self.assertEqual(len(builtins),len(m.defaults()))
        item=builtins[0];path=m.audio_path(item)
        with self.request('/api/delete',json.dumps({'id':item['id']}).encode()) as r:self.assertTrue(json.load(r)['ok'])
        self.assertNotIn(item['id'],[f['id'] for f in m.library()])
        self.assertTrue(path.exists())
        with self.request('/api/restore',b'{}') as r:self.assertTrue(json.load(r)['ok'])
        self.assertEqual(sum(f.get('builtin',False) for f in m.library()),len(m.defaults()))
    def test_empty_library_remains_empty(self):
        previous=m.library()
        m.save_library([])
        self.assertEqual(m.library(),[])
        m.save_library(previous)
    def test_formats(self):
        for ext in ['mp3','wav','m4a','aac','flac','ogg','opus','aiff','wma']:
            with self.subTest(ext=ext):
                f=m.DATA/('fixture.'+ext)
                subprocess.run(['ffmpeg','-v','error','-f','lavfi','-i','sine=frequency=440:duration=0.3','-y',str(f)],check=True)
                with self.request('/api/upload?name=sample.'+ext,f.read_bytes()) as r: self.assertTrue(json.load(r)['ok'])
                item=m.library()[-1]
                with self.request('/audio/'+item['id']) as r:self.assertEqual(r.read(4),b'RIFF')
                with self.request('/api/delete',json.dumps({'id':item['id']}).encode()) as r:self.assertTrue(json.load(r)['ok'])
    def test_reject_invalid(self):
        for name in ['bad.exe','bad.mp3']:
            with self.assertRaises(HTTPError) as ctx:self.request('/api/upload?name='+name,b'not audio')
            self.assertEqual(ctx.exception.code,400)
        self.assertFalse(m.STATE['busy'])
    def test_token(self):
        with self.assertRaises(HTTPError) as ctx:self.request('/api/build',b'{}',False)
        self.assertEqual(ctx.exception.code,403)
if __name__=='__main__':unittest.main()
