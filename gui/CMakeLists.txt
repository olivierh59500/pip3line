####
# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information
####

message ( STATUS "Configuring pip3line gui --" )
project( pip3line )
set ( CMAKE_BUILD_TYPE Release )

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (WITH_SCINTILLA)
	message ( STATUS "-- Enabling QScintilla support --" )
        add_definitions ( -DSCINTILLA )
endif(WITH_SCINTILLA)

if(WITH_QT5)
        find_package(Qt5Widgets)
        find_package(Qt5Core)
        find_package(Qt5Svg)
        find_package(Qt5Network)
	find_package(Qt5Concurrent)
else(WITH_QT5)
	find_package ( Qt4 REQUIRED QtCore QtGui QtSvg QtNetwork QtMain )
	include ( ${QT_USE_FILE} )
endif(WITH_QT5)

include_directories (
	../libtransform)

if (NOT APPLE)
  link_directories (
    ${PROJECT_SOURCE_DIR}/../lib
    /usr/lib64/qt4/
  )
else()
	message( STATUS "-- Enabling MAC OSX specifics --" )
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.perso.pip3line")
	set(APP_TYPE MACOSX_BUNDLE)
endif()

#set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin )

set ( EXTERN_LIBS transform )

if (WITH_SCINTILLA)
        set ( EXTERN_LIBS ${EXTERN_LIBS} qscintilla2)
endif(WITH_SCINTILLA)

if (UNIX)
	add_definitions ( -Wall )
endif (UNIX)

set ( pip3line_HDRS
	confgui/transformguimgmt.h
	mainwindow.h
	transformwidget.h
	aboutdialog.h
	messagedialog.h
	analysedialog.h
	massprocessingdialog.h
	regexphelpdialog.h
	loggerwidget.h
	infodialog.h
	screeniodevice.h
	settingsdialog.h
	downloadmanager.h
	pluginconfwidget.h
	guihelper.h
	maintabs.h
	floatingdialog.h
	quickviewdialog.h
	quickviewitem.h
	quickviewitemconfig.h
	newbytedialog.h
	debugdialog.h
	textinputdialog.h
	comparisondialog.h
	appdialog.h
	../tools/processor.h
	../tools/pipeserver.h
	../tools/streamprocessor.h
	../tools/tcpserver.h
	../tools/binaryprocessor.h
	../tools/textprocessor.h
	../tools/serverabstract.h
	../tools/processingstats.h
	../version.h
	sources/basicsource.h
	sources/bytesourceabstract.h
	sources/currentmemorysource.h
	sources/largerandomaccesssource.h
	sources/memorywidget.h
	sources/filewidget.h
	sources/largefile.h
	sources/searchabstract.h
	sources/intermediatesource.h
	sources/blocksources/blockssource.h
	sources/blocksources/ipblockssources.h
	sources/blocksources/connectionsinfoswidget.h
	sources/blocksources/connectiondetails.h
	sources/blocksources/connectionswidget.h
	sources/blocksources/target.h
	sources/blocksources/udpclientlistener.h
	sources/blocksources/ipnetworkserverwidget.h
	sources/blocksources/udpserverlistener.h
	sources/blocksources/tlsserverlistener.h
	sources/blocksources/baseblocksourcewidget.h
	sources/blocksources/ipnetworkclientwidget.h
	sources/blocksources/tlsclientlistener.h
	sources/blocksources/socksproxyhelper.h
	views/hexview.h
	views/textview.h
	views/bytetableview.h
	views/byteitemmodel.h
	views/singleviewabstract.h
	views/foldedview.h
	tabs/tababstract.h
	tabs/transformsgui.h
	tabs/generictab.h
	tabs/tababstract.h
	tabs/transformsgui.h
	tabs/randomaccesstab.h
	tabs/packetanalysertab.h
	tabs/packetanalyser/importexportdialog.h
	tabs/packetanalyser/importexportworker.h
	tabs/packetanalyser/packet.h
	tabs/packetanalyser/memorypacketmodel.h
	tabs/packetanalyser/packetanalyseroptionsdialog.h
	tabs/packetanalyser/packetmodelabstract.h
	tabs/packetanalyser/packetsortfilterproxymodel.h
	tabs/packetanalyser/packetstyleditemdelegate.h
	tabs/packetanalyser/sourcesorchestatorabstract.h
	tabs/packetanalyser/orchestratorchooser.h
	tabs/packetanalyser/externalproxyorchestrator.h
	tabs/packetanalyser/proxyorchestrator.h
	tabs/packetanalyser/socksorchestrator.h
	shared/offsetgotowidget.h
	shared/searchwidget.h
	shared/readonlybutton.h
	shared/clearallmarkingsbutton.h
	shared/bytesourceguibutton.h
	shared/detachtabbutton.h
	shared/universalreceiverbutton.h
	shared/messagepanelwidget.h
	shared/searchresultswidget.h
	shared/newviewbutton.h
	shared/newviewmenu.h
	shared/guiconst.h
	shared/sslconf.h
	shared/ssloptionswidget.h
	shared/defaultcontrolgui.h
	shared/defaultdialog.h
	shared/sendtomenu.h
	shared/transformselectorwidget.h
	state/basestateabstract.h
	state/stateorchestrator.h
	state/closingstate.h
	state/statedialog.h
	state/statestatuswidget.h
	pcapio/pcapngio.h
	pcapio/pcapio.h
	pcapio/pcapdef.h
	)


set ( pip3line_SRCS
	main.cpp
	win.rc
	mainwindow.cpp
	transformwidget.cpp
	messagedialog.cpp
	aboutdialog.cpp
	analysedialog.cpp
	massprocessingdialog.cpp
	regexphelpdialog.cpp
	loggerwidget.cpp
	infodialog.cpp
	screeniodevice.cpp
	settingsdialog.cpp
	downloadmanager.cpp
	pluginconfwidget.cpp
	guihelper.cpp
	maintabs.cpp
	floatingdialog.cpp
	quickviewdialog.cpp
	quickviewitem.cpp
	quickviewitemconfig.cpp
	newbytedialog.cpp
	debugdialog.cpp
	textinputdialog.cpp
	comparisondialog.cpp
	appdialog.cpp
	../tools/processor.cpp
	../tools/binaryprocessor.cpp
	../tools/textprocessor.cpp
	../tools/pipeserver.cpp
	../tools/streamprocessor.cpp
	../tools/tcpserver.cpp
	../tools/serverabstract.cpp
	../tools/processingstats.cpp
	sources/basicsource.cpp
	sources/bytesourceabstract.cpp
	sources/currentmemorysource.cpp
	sources/largerandomaccesssource.cpp
	sources/memorywidget.cpp
	sources/filewidget.cpp
	sources/largefile.cpp
	sources/searchabstract.cpp
	sources/intermediatesource.cpp
	sources/blocksources/blockssource.cpp
	sources/blocksources/ipblockssources.cpp
	sources/blocksources/connectionsinfoswidget.cpp
	sources/blocksources/connectiondetails.cpp
	sources/blocksources/connectionswidget.cpp
	sources/blocksources/udpclientlistener.cpp
	sources/blocksources/ipnetworkserverwidget.cpp
	sources/blocksources/udpserverlistener.cpp
	sources/blocksources/tlsserverlistener.cpp
	sources/blocksources/baseblocksourcewidget.cpp
	sources/blocksources/ipnetworkclientwidget.cpp
	sources/blocksources/tlsclientlistener.cpp
	sources/blocksources/socksproxyhelper.cpp
	views/hexview.cpp
	views/bytetableview.cpp
	views/byteitemmodel.cpp
	views/textview.cpp
	views/singleviewabstract.cpp
	views/foldedview.cpp
	tabs/generictab.cpp
	tabs/tababstract.cpp
	tabs/transformsgui.cpp
	tabs/transformsgui.cpp
	tabs/tababstract.cpp
	tabs/randomaccesstab.cpp
	tabs/packetanalysertab.cpp
	tabs/packetanalyser/importexportdialog.cpp
	tabs/packetanalyser/importexportworker.cpp
	tabs/packetanalyser/packet.cpp
	tabs/packetanalyser/memorypacketmodel.cpp
	tabs/packetanalyser/packetanalyseroptionsdialog.cpp
	tabs/packetanalyser/packetmodelabstract.cpp
	tabs/packetanalyser/packetsortfilterproxymodel.cpp
	tabs/packetanalyser/packetstyleditemdelegate.cpp
	tabs/packetanalyser/sourcesorchestatorabstract.cpp
	tabs/packetanalyser/singlesourceorchestrator.cpp
	tabs/packetanalyser/orchestratorchooser.cpp
	tabs/packetanalyser/externalproxyorchestrator.cpp
	tabs/packetanalyser/proxyorchestrator.cpp
	tabs/packetanalyser/socksorchestrator.cpp
	shared/offsetgotowidget.cpp
	shared/searchwidget.cpp
	shared/readonlybutton.cpp
	shared/clearallmarkingsbutton.cpp
	shared/bytesourceguibutton.cpp
	shared/detachtabbutton.cpp
	shared/universalreceiverbutton.cpp
	shared/messagepanelwidget.cpp
	shared/searchresultswidget.cpp
	shared/newviewbutton.cpp
	shared/newviewmenu.cpp
	shared/guiconst.cpp
	shared/sslconf.cpp
	shared/ssloptionswidget.cpp
	shared/defaultcontrolgui.cpp
	shared/defaultdialog.cpp
	shared/sendtomenu.cpp
	shared/transformselectorwidget.cpp
	state/basestateabstract.cpp
	state/stateorchestrator.cpp
	state/closingstate.cpp
	state/statedialog.cpp
	state/statestatuswidget.cpp
	pcapio/pcapdef.cpp
	pcapio/pcapio.cpp
	pcapio/pcapngio.cpp
	)

set ( pip3line_UIS
	mainwindow.ui
	comparisondialog.ui
	transformwidget.ui
	aboutdialog.ui
	messagedialog.ui
	regexphelpdialog.ui
	analysedialog.ui
	loggerwidget.ui
	massprocessingdialog.ui
	insertdialog.ui
	infodialog.ui
	settingsdialog.ui
	pluginconfwidget.ui
	comparisondialog.ui
	textinputdialog.ui
	tabs/transformsgui.ui
	tabs/generictab.ui
	tabs/transformsgui.ui
	tabs/randomaccesstab.ui
	tabs/packetanalysertab.ui
	tabs/packetanalyser/importexportdialog.ui
	tabs/packetanalyser/packetanalyseroptionsdialog.ui
	floatingdialog.ui
	quickviewdialog.ui
	quickviewitem.ui
	quickviewitemconfig.ui
	newbytedialog.ui
	debugdialog.ui
	sources/memorywidget.ui
	sources/filewidget.ui
	sources/blocksources/connectionsinfoswidget.ui
	sources/blocksources/connectionswidget.ui
	sources/blocksources/ipnetworkserverwidget.ui
	sources/blocksources/baseblocksourcewidget.ui
	sources/blocksources/ipnetworkclientwidget.ui
	shared/searchresultswidget.ui
	shared/transformdisplayconf.ui
	shared/ssloptionswidget.ui
	shared/messagepanelwidget.ui
	shared/defaultcontrolgui.ui
	shared/defaultdialog.ui
	shared/transformselectorwidget.ui
	state/statedialog.ui
	state/statestatuswidget.ui
	views/hexview.ui
	views/textview.ui
	views/foldedview.ui
	)

set ( pip3line_RSCS
	gui_res.qrc
	)

if(WITH_QT5)
        qt5_wrap_ui(UIS ${pip3line_UIS})
	qt5_add_resources(RSCS ${pip3line_RSCS})
else(WITH_QT5)
        QT4_WRAP_UI(UIS ${pip3line_UIS})
	QT4_ADD_RESOURCES(RSCS ${pip3line_RSCS})
endif(WITH_QT5)

add_executable ( pip3line WIN32 ${pip3line_SRCS} ${UIS} ${RSCS})

if(WITH_QT5)
        qt5_use_modules(pip3line Widgets Svg Network Concurrent)
	if(WIN32)
		target_link_libraries ( pip3line Qt5::WinMain )
	endif(WIN32)
else(WITH_QT5)
       	target_link_libraries ( pip3line  ${QT_LIBRARIES} )
endif(WITH_QT5)

target_link_libraries ( pip3line ${EXTERN_LIBS} )

if(UNIX)
	install (TARGETS pip3line DESTINATION bin)
	install (FILES pip3line.desktop DESTINATION /usr/share/applications)
	install (FILES icons/pip3line.png DESTINATION /usr/share/icons/hicolor/128x128/apps)
endif(UNIX)

