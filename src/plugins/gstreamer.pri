unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += gstreamer-0.10
}
win32 {
	INCLUDEPATH +=	$(OSSBUILD_GSTREAMER_SDK_DIR)/include \
					$(OSSBUILD_GSTREAMER_SDK_DIR)/include/gstreamer-0.10 \
					$(OSSBUILD_GSTREAMER_SDK_DIR)/include/glib-2.0 \
					$(OSSBUILD_GSTREAMER_SDK_DIR)/include/libxml2

	LIBS +=			-L$(OSSBUILD_GSTREAMER_SDK_DIR)/lib \
					-lgstreamer-0.10 \
					-lglib-2.0 \
					-liconv \
					-lxml2 \
					-lgobject-2.0
}

# vim: set ts=4 sw=4: #
