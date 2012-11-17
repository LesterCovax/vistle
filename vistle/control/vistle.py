import sys;
import _vistle;
from _vistle import *;

# print to network clients
class _stdout:
   def write(self, stuff):
      _vistle._print_output(stuff)

class _stderr:
   def write(self, stuff):
      _vistle._print_error(stuff)

class _stdin:
   def read(self, stuff):
      _vistle._read_input()

sys.stdout = _stdout()
sys.stderr = _stderr()
#sys.stdin = _stdin()

# redefine help
python_help = help
def help():
   current_module = sys.modules[__name__]
   python_help(current_module)

