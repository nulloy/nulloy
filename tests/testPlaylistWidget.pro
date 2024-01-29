include(test.pri)
QT += testlib

TARGET = testPlaylistWidget
SOURCES += testPlaylistWidget.cpp

# generate 10 seconds samples:
system(gst-launch-1.0 audiotestsrc samplesperbuffer=441 num-buffers=1000 ! audioconvert ! wavenc ! filesink location=sine.wav)
LIST = 01 02 03 04 05 06 07 08 09 10
for(index, LIST){
    system($$QMAKE_COPY sine.wav $${index}.wav)
}
