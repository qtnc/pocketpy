path.sep = '/'
path.extsep = '.'

def _join (*args):
  return path.sep.join(args)

def _basename (p):
  return p[p.rfind(path.sep)+1:]

def _dirname (p):
  return p[:p.rfind(path.sep)]

def _split (p):
  return (_dirname(p), _basename(p))

def _splitext (p):
  i = p.rfind(path.extsep)
  return (p[:i], p[i:])

path.join = _join
path.basename = _basename
path.dirname = _dirname
path.split = _split
path.splitext = _splitext
