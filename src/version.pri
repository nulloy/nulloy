isEmpty(N_CONFIG_FORCE_VERSION) {
	N_VERSION = $$system($$PWD/version-git.sh)
} else {
	N_VERSION = $$N_CONFIG_FORCE_VERSION
}
