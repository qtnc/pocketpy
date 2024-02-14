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


def combinations(iterable, r):
    pool = tuple(iterable)
    n = len(pool)
    if r > n: return
    indices = list(range(r))
    yield tuple(pool[i] for i in indices)
    while True:
        for i in range(r -1, -1, -1):
            if indices[i] != i + n - r: break
        else: return
        indices[i] += 1
        for j in range(i+1, r):
            indices[j] = indices[j-1] + 1
        yield tuple(pool[i] for i in indices)

def combinations_with_replacement(iterable, r):
    pool = tuple(iterable)
    n = len(pool)
    if not n and r: return
    indices = [0] * r
    yield tuple(pool[i] for i in indices)
    while True:
        for i in range(r -1, -1, -1):
            if indices[i] != n - 1: break
        else: return
        indices[i:] = [indices[i] + 1] * (r - i)
        yield tuple(pool[i] for i in indices)

def permutations(iterable, r=None):
    pool = tuple(iterable)
    n = len(pool)
    r = n if r is None else r
    if r > n: return
    indices = list(range(n))
    cycles = list(range(n, n-r, -1))
    yield tuple(pool[i] for i in indices[:r])
    while n:
        for i in range(r -1, -1, -1):
            cycles[i] -= 1
            if cycles[i] == 0:
                indices[i:] = indices[i+1:] + indices[i:i+1]
                cycles[i] = n - i
            else:
                j = cycles[i]
                indices[i], indices[-j] = indices[-j], indices[i]
                yield tuple(pool[i] for i in indices[:r])
                break
        else:
            return
