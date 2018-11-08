cmake ^
	-H. ^
	-B"%BuildDirectory%" ^
	-G"%Generator%" ^
	-DCMAKE_INSTALL_PREFIX="%FullDistributePath%\node-fontinfo"

cmake --build %BuildDirectory% --target install --config RelWithDebInfo