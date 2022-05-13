class export:
    pass


class unit(type):
    def __new__(mcs, name, bases, namespace):
        ano: dict = namespace["__annotations__"]
        names = set(ano.keys()) | namespace["__slots__"] | set(namespace.keys())
        exports = filter(namespace)

class test(metaclass=unit):
    a: int
