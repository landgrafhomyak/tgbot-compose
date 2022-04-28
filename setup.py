from setuptools import setup, Extension

setup(
    name="tgbot_compose",
    packages=["tgbot_compose"],
    package_data={"tgbot_compose": ["py.typed", "api_types.pyi"]},
    package_dir={"tgbot_compose": "src"},
    ext_package="tgbot_compose",
    ext_modules=[
        Extension(
            name="api_types",
            sources=[
                "src/api_types/__init__.c",
                "src/api_types/property.c",
                "src/api_types/User.c"
            ]
        )
    ]
)