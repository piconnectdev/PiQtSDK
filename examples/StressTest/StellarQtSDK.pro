QT += core network
QT -= gui


TEMPLATE = lib

CONFIG += c++11 console
CONFIG -= app_bundle
CONFIG += staticlib

TARGET = StellarQtSDK

include($$PWD/../../StellarQtSDK.pri)

DESTDIR = ../../bin/

target.path = $$DESTDIR/
