include $(top_srcdir)/generic.am

install-exec-local:
	mkdir -p $(USR_DIR)/share
	cp -r fonts $(USR_DIR)/share/
	cp -r icons $(USR_DIR)/share/ # cp -r wallpapers $(USR_DIR)/share/
	mkdir -p $(HOME_DIR)
	cp home/* $(HOME_DIR)/

uninstall-local:
	rm -rf $(USR_DIR)/share/
	rm -rf $(HOME_DIR)/

all-local: $(OBJECT_NAME)

clean-local:
	rm -f $(OBJ) $(OBJECT_NAME)
