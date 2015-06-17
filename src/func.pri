defineReplace(fixSlashes) {
	win32:!unix_mingw:1 ~= s,/,\\,g
	return($$1)
}
