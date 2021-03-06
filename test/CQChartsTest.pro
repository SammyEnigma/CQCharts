TEMPLATE = app

TARGET = CQChartsTest

QT += widgets svg xml

DEPENDPATH += .

QMAKE_CXXFLAGS += \
-std=c++14 \
-DCQCHARTS_FOLDED_MODEL \

MOC_DIR = .moc

SOURCES += \
CQAppWindow.cpp \
CQChartsAppWindow.cpp \
CQChartsCmdArg.cpp \
CQChartsCmdArgs.cpp \
CQChartsCmdBase.cpp \
CQChartsCmdGroup.cpp \
CQChartsCmds.cpp \
CQChartsCmdsSlot.cpp \
CQChartsInitData.cpp \
CQChartsReadLine.cpp \
CQChartsTest.cpp \
\
CQTclCmd.cpp \

HEADERS += \
CQAppWindow.h \
CQChartsAppWindow.h \
CQChartsCmdArg.h \
CQChartsCmdArgs.h \
CQChartsCmdBase.h \
CQChartsCmdGroup.h \
CQChartsCmds.h \
CQChartsCmdsSlot.h \
CQChartsInitData.h \
CQChartsInput.h \
CQChartsNameValueData.h \
CQChartsReadLine.h \
CQChartsTest.h \
\
CQTclCmd.h \

DESTDIR     = ../bin
OBJECTS_DIR = ../obj

INCLUDEPATH += \
. \
../include \
../../CQPropertyView/include \
../../CQBaseModel/include \
../../CQColors/include \
../../CQUtil/include \
../../CQPerfMonitor/include \
../../CImageLib/include \
../../CFont/include \
../../CReadLine/include \
../../CFile/include \
../../CMath/include \
../../CStrUtil/include \
../../CUtil/include \
../../COS/include \
/usr/include/tcl \

PRE_TARGETDEPS = \
../lib/libCQCharts.a \
../../CQPropertyView/lib/libCQPropertyView.a \
../../CQModelView/lib/libCQModelView.a \
../../CQColors/lib/libCQColors.a \

unix:LIBS += \
-L../lib \
-L../../CQPropertyView/lib \
-L../../CQModelView/lib \
-L../../CQBaseModel/lib \
-L../../CQColors/lib \
-L../../CQCustomCombo/lib \
-L../../CQUtil/lib \
-L../../CJson/lib \
-L../../CCsv/lib \
-L../../CXML/lib \
-L../../CQPerfMonitor/lib \
-L../../CConfig/lib \
-L../../CImageLib/lib \
-L../../CFont/lib \
-L../../CMath/lib \
-L../../CReadLine/lib \
-L../../CFileUtil/lib \
-L../../CFile/lib \
-L../../CUtil/lib \
-L../../CRegExp/lib \
-L../../CStrUtil/lib \
-L../../COS/lib \
-lCQCharts -lCQPropertyView -lCQModelView -lCQBaseModel -lCQColors \
-lCQCustomCombo -lCQUtil -lCQPerfMonitor -lCJson -lCCsv \
-lCXML -lCConfig -lCImageLib -lCFont -lCMath \
-lCReadLine -lCFileUtil -lCFile -lCRegExp \
-lCUtil -lCStrUtil -lCOS \
-lreadline -lpng -ljpeg -ltre -ltcl
