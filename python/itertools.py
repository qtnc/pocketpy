def accumulate (it, func=None, initial=None):
  if func is None: func = lambda a, b: a+b
  total = initial
  for e in it:
    total = e if total is None else func(total, e)
    yield total

def chain (*its):
  for it in its: for e in it: yield e

def compress (data, selectors):
  return (d for d, s in zip(data, selectors) if s)

def count (start=0, step=1):
  while True:
    yield start
    start += step

def islice (it, start, stop=None, step=1):
  if stop is None: start, stop = 0, start
  if start>0: for i in range(start): next(it)
  while start<stop:
    yield next(it)
    if step>1: for i in range(step -1): next(it)
    start += step

def cycle (it):
  if type(it) not in (list, tuple): it = [x for x in it]
  while True: for e in it: yield e

def dropwhile (p, it, *a):
  it = iter(it)
  for e in it: if not p(e, *a):
    yield e; break
  for e in it: yield e

def takewhile (p, it, *a):
  for e in it:
    if p(e, *a): yield e
    else: break

def filterfalse (p, it, *a):
  for e in it: if not p(e, *a): yield e

def pairwise (it, n=2):
  l = []
  for e in it:
    if len(l)>=n: del l[0]
    l.append(e)
    if len(l)==n: yield tuple(l)

def batched (it, n):
  l = []
  for e in it:
    l.append(e)
    if len(l)==n:
      yield tuple(l)
      l.clear()
  if l: yield tuple(l)

def product (*its, repeat=1):
  if repeat>1: its = repeat * list(its)
  if len(its)<=0: yield []
  else: for e in its[0]: for g in product(*its[1:]): yield [e] + g

def repeat (e, n=None):
  if n is None:
    while True: yield e
  else:
    for i in range(n): yield e

def starmap (f, it):
  for a in it: yield f(*a)

def zip(*args):
    its = [iter(x) for x in args]
    while True:
        l = [next(x) for x in its]
        if StopIteration in l: break
        yield l

def zip_longest (*args, fillvalue=None):
    its = [iter(x) for x in args]
    argc = len(its)
    while True:
        l = [next(x) for x in its]
        c = 0
        for i in range(argc):
            if l[i] is StopIteration:
                l[i] = fillvalue
                ++c
        if c<argc: yield l
        else: break
