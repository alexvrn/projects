######################################################################
# Automatically generated by qmake (2.01a) ?? ???. 11 12:18:08 2013
######################################################################

TEMPLATE = app
TARGET = GDStorageFileUploader

QT += widgets core gui uitools network xml

# Input
HEADERS += View/addfilespanelview.h \
           View/mainwindowview.h \
           View/uploadprogressview.h \
    Service/dialogservice.h \
    Presenter/addfilespresenter.h \
    View/iview.h \
    View/iviewadd.h \
    View/iviewupload.h \
    View/iviewmain.h \
    Model/fileinfomodel.h \
    Model/fileshortinfotablemodel.h \
    Model/fileshortinfomodel.h \
    Model/fileuploadinfotablemodel.h \
    Model/fileuploadinfomodel.h \
    Presenter/uploadprogresspresenter.h \
    View/passworddialogview.h \
    View/iviewdata.h \
    Service/credentials.h \
    Presenter/mainwindowpresenter.h \
    Presenter/passworddialogpresenter.h \
    View/iviewdialogpassword.h \
    Service/serviceclient.h \
    Model/fileshortinfodelegate.h \
    Model/fileuploadinfodelegate.h \
    Service/messageoutput.h \
    Service/serialization.h \
    Model/addfilesmodel.h \
    Model/passworddialogmodel.h \
    Model/uploadprogressmodel.h \
    protocol/remoteclient.pb.h \
    Model/mainwindowmodel.h \
    View/splitbutton.h \
    Service/setting.h \
    Service/RemoteDialog/filesystemremotecontroller.h \
    Service/RemoteDialog/filesystemremotedialog.h \
    Service/RemoteDialog/filesystemremotemodel.h

FORMS +=

SOURCES += View/addfilespanelview.cpp \
           main.cpp \
           View/mainwindowview.cpp \
           View/uploadprogressview.cpp \
    Service/dialogservice.cpp \
    Presenter/addfilespresenter.cpp \
    Model/fileshortinfotablemodel.cpp \
    Model/fileshortinfomodel.cpp \
    Model/fileuploadinfotablemodel.cpp \
    Presenter/uploadprogresspresenter.cpp \
    View/passworddialogview.cpp \
    View/iviewdata.cpp \
    Service/credentials.cpp \
    Presenter/mainwindowpresenter.cpp \
    Presenter/passworddialogpresenter.cpp \
    Service/serviceclient.cpp \
    Model/fileshortinfodelegate.cpp \
    Model/fileuploadinfomodel.cpp \
    Model/fileuploadinfodelegate.cpp \
    Service/serialization.cpp \
    Model/addfilesmodel.cpp \
    Model/passworddialogmodel.cpp \
    Model/uploadprogressmodel.cpp \
    protocol/remoteclient.pb.cc \
    Model/mainwindowmodel.cpp \
    View/splitbutton.cpp \
    Service/setting.cpp \
    Service/RemoteDialog/filesystemremotecontroller.cpp \
    Service/RemoteDialog/filesystemremotedialog.cpp \
    Service/RemoteDialog/filesystemremotemodel.cpp



INCLUDEPATH += C:\QtProject\GDStorageFileUploader\external\libprotobuf\src

LIBS += C:\QtProject\GDStorageFileUploader\external\build\libprotobuf\libprotobuf\libprotobuf.a

CONFIG += c++11

TRANSLATIONS += ru.ts

OTHER_FILES += \
    protobuf.pri \
    protocol/remoteclient.proto

RESOURCES += \
    resources.qrc
