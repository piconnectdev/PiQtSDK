TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS += $$PWD/StellarQtSDK.pro
SUBDIRS += $$PWD/StressTest.pro

StressTest.depends = StellarQtSDK
