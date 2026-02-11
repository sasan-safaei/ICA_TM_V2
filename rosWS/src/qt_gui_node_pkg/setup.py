from setuptools import find_packages, setup

package_name = 'qt_gui_node_pkg'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    package_data={ 'qt_gui_node_pkg': ['ui/py/*.py'],},
    include_package_data=True,
    data_files=[
        ('share/ament_index/resource_index/packages',['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools', 'rclpy', 'PyQt5'],
    zip_safe=True,
    maintainer='ica',
    maintainer_email='sasan.safaei@ica.de',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'gui_node = qt_gui_node_pkg.gui_node:main',
        ],
    },
    

)
