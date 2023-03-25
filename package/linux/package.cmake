cmake_minimum_required(VERSION 3.11)

set(CPACK_PACKAGE_DESCRIPTION "domain block service")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "域名阻断的服务程序")
set(CPACK_PACKAGE_VENDOR "da234cao")
set(CPACK_PACKAGE_CONTACT "da1234cao.github.io")
set(CPACK_PACKAGE_NAME "domain_block_service")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)

set(INSTALL_DIR /opt/domain_block)
set(SYSTEMD_DIR /etc/systemd/system/)

# 服务程序
install(TARGETS domain_block_service RUNTIME DESTINATION ${INSTALL_DIR})
# 服务端配置文件-把配置文件和可执行程序防在一起，不符合fhs
install(FILES  ${CMAKE_SOURCE_DIR}/linux/domain_block_service.toml DESTINATION ${INSTALL_DIR})

# 客户端程序
# 客户端配置文件-执行的时候会复制一份到家目录的.config文件
install(TARGETS domain_block_client RUNTIME DESTINATION ${INSTALL_DIR})
install(FILES ${CMAKE_SOURCE_DIR}/UI/domain_block_client/domain_block_client.ini DESTINATION ${INSTALL_DIR})

# systemctl配置文件
install(FILES ${CMAKE_SOURCE_DIR}/linux/domain_block.service DESTINATION ${SYSTEMD_DIR})

set(CPACK_GENERATOR "DEB")

# 安装/卸载前后的执行脚本
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CMAKE_SOURCE_DIR}/package/linux/postinst;
                                       ${CMAKE_SOURCE_DIR}/package/linux/prerm;
                                       ${CMAKE_SOURCE_DIR}/package/linux/postrm)

# 程序发布的时候没有携带qt库，需要用户自行安装
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt5-default")

# 忽略所有路径中包含"toml"的文件
set(CPACK_SOURCE_IGNORE_FILES
    /.*toml.*
)

include(CPack)

