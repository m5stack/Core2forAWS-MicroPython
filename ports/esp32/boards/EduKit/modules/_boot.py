import gc
import uos
from flashbdev import bdev

try:
    if bdev:
        vfs = uos.VfsLfs2(bdev)
        vfs = uos.VfsLfs2(bdev, progsize=32, readsize=128, lookahead=128)
        uos.mount(vfs, "/flash")
except OSError:
    uos.VfsLfs2.mkfs(bdev)
    vfs = uos.VfsLfs2(bdev, progsize=32, readsize=128, lookahead=128)
    uos.mount(vfs, "/flash")

uos.chdir('/flash')
gc.collect()

