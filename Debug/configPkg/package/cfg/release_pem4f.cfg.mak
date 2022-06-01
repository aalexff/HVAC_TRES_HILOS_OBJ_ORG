# invoke SourceDir generated makefile for release.pem4f
release.pem4f: .libraries,release.pem4f
.libraries,release.pem4f: package/cfg/release_pem4f.xdl
	$(MAKE) -f C:\Users\Administrador\workspace_v11\HVAC_TRES_HILOS_OBJ_ORG\Aux_files/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\Administrador\workspace_v11\HVAC_TRES_HILOS_OBJ_ORG\Aux_files/src/makefile.libs clean

