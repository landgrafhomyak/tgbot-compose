from platform import python_implementation

kwargs = dict(
    name="tgbot_compose",
    packages=["tgbot_compose"],
    package_data={"tgbot_compose": ["py.typed", "api_types.pyi", "meta.pyi"]},
    package_dir={"tgbot_compose": "src"},
    ext_package="tgbot_compose"
)

if False and python_implementation() == "CPython":
    from skbuild import setup
    kwargs                                             \
        .setdefault("exclude_package_data", dict())    \
        .setdefault("tgbot_compose", [])               \
        .extend(["meta.py"])
else:
    from setuptools import setup

setup(**kwargs)
