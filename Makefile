include $(TOPDIR)/rules.mk

PKG_NAME:=pushprox-client
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_MAINTAINER:=John Doe <john.doe@example.com>
PKG_LICENSE:=CC0-1.0

include $(INCLUDE_DIR)/package.mk

define Package/pushprox-client
	SECTION:=utils
	# Select package by default
	#DEFAULT:=y
	CATEGORY:=Utilities
	TITLE:=pushprox-client.
	DEPENDS:=+libcurl
	URL:=https://github.com/prometheus-community/PushProx
endef

define Package/pushprox-client/description
	Simple client for prometheus pushprox proxy.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
# Nothing to do here for us.
# By default pushprox-client/src/Makefile will be used.
endef

define Build/Compile
	CFLAGS="$(TARGET_CFLAGS)" CPPFLAGS="$(TARGET_CPPFLAGS)" $(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef

define Package/pushprox-client/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pushprox-client $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,pushprox-client))
