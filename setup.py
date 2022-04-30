from skbuild import setup

setup(
    name="tgbot_compose",
    packages=["tgbot_compose"],
    package_data={"tgbot_compose": ["py.typed", "api_types.pyi"]},
    package_dir={"tgbot_compose": "src"},
    ext_package="tgbot_compose",
)
