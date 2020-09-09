from setuptools import setup

setup(
   name='mlinwrf',
   version='1.0',
   description='A module to run trained machine learning models within WRF.',
   author='David John Gagne',
   author_email='dgagne@ucar.edu',
   packages=['mlinwrf'],
   install_requires=['numpy', 'pandas', 'tensorflow', 'xarray'],
)
