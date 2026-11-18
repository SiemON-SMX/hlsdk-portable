#
# Copyright (c) 2015 Pavlo Lavrenenko
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION 3.9)
project (SVDLL)

set (SVDLL_LIBRARY server)

if(NOT MSVC)
	add_compile_options(-fno-exceptions) # GCC/Clang flag
	add_compile_options(-fno-rtti) # GCC/Clang flag
	add_compile_options(-Wno-invalid-offsetof) # GCC/Clang flag
	add_compile_options(-fvisibility=hidden) # GCC/Clang flag
	add_definitions(-D_LINUX) # It seems enough for all non-Win32 systems
	add_definitions(-Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf -D_vsnprintf=vsnprintf )
	if(CMAKE_BUILD_TYPE MATCHES "Release"
		OR (CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo"
		AND CMAKE_SYSTEM_NAME STREQUAL "Android"))
		add_compile_options(-fno-unwind-tables -fno-asynchronous-unwind-tables) # GCC/Clang flag
		add_compile_options(-fomit-frame-pointer) # GCC/Clang flag
		add_compile_options(-ftree-vectorize) # GCC/Clang flag
		add_compile_options(-funsafe-math-optimizations) # GCC/Clang flag
	endif()
else()
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE)
endif()

set (SVDLL_SOURCES
	AI_BaseNPC_Schedule.cpp
	alias.cpp
	animating.cpp
	animation.cpp
	bmodels.cpp
	buttons.cpp
	cbase.cpp
	client.cpp
	3DKeypad.cpp
	Action.cpp
	AnimatedCamera.cpp
	Axe.cpp
	BarShoot.cpp
	Boat.cpp
	BookLaser.cpp
	BookSimonSledgehammer.cpp
	bowlingball.cpp
	Branch.cpp
	wheelchair.cpp
	Camera.cpp
	ChainsawBoss.cpp
	CheckBrush.cpp
	CoopButtonPress.cpp
	CoopLevelChangeManager.cpp
	Crab.cpp
	CutsceneModel.cpp
	DeveloperCommentary.cpp
	DoctorBoss.cpp
	DoctorBossShoot.cpp
	Door_Prop_View.cpp
	dualweapon.cpp
	DualPistol.cpp
	Faceless.cpp
	FacelessValve.cpp
	FAMAS.cpp
	Faster.cpp
	Flare.cpp
	Flashlight.cpp
	Flygare.cpp
	FuncValve.cpp
	G43.cpp
	Glock.cpp
	InterDoor.cpp
	InterDoorCoop.cpp
	KeyItem.cpp
	keys.cpp
	Ladder.cpp
	Lantern.cpp
	M16.cpp
	MobilePhone.cpp
	MonsterBaby.cpp
	MonsterChild.cpp
	MonsterCrazyBitch.cpp
	MonsterCustom.cpp
	MonsterKrypande.cpp
	MonsterSewmo.cpp
	MonsterSlower3.cpp
	mp5.cpp
	NiceCamera.cpp
	NightStick.cpp
	NumeralPlate.cpp
	P345.cpp
	PhoneCall.cpp
	PushableVan.cpp
	PuzzleBars.cpp
	PuzzleBarsButton.cpp
	Radio.cpp
	Revolver.cpp
	Rifle.cpp
	RoofBoss.cpp
	RunningCrazy.cpp
	SawCrazy.cpp
	SawRunner.cpp
	SewerBoss.cpp
	Shotgun.cpp
	SickSimon.cpp
	Sledgehammer.cpp
	Slower.cpp
	SlowerTentacled.cpp
	Spitter.cpp
	Statues.cpp
	StatueUse.cpp
	Stranger.cpp
	Strangle.cpp
	SubtitleChange.cpp
	SubtitleMain.cpp
	SubwayBrickWall.cpp
	Suicider.cpp
	Switchblade.cpp
	Syringe.cpp
	Taller.cpp
	TapeRecorder.cpp
	Telephone.cpp
	Telescope.cpp
	ThirdPersonInteraction.cpp
	TMP.cpp
	Twitcher.cpp
	Twitcher2.cpp
	Twitcher3.cpp
	Twitcher4.cpp
	UserPasswordNote.cpp
	VP70.cpp
	Watro.cpp
	WheelchairPlayer.cpp
	combat.cpp
	defaultai.cpp
	doors.cpp
	effects.cpp
	explode.cpp
	flyingmonster.cpp
	func_break.cpp
	game.cpp
	gamerules.cpp
	genericmonster.cpp
	ggrenade.cpp
	globals.cpp
	h_ai.cpp
	h_battery.cpp
	h_cine.cpp
	h_cycler.cpp
	h_export.cpp
	item_padlock.cpp
	items.cpp
	lights.cpp
	locus.cpp
	maprules.cpp
	monster_ruben.cpp
	monstermaker.cpp
	monsters.cpp
	monsterstate.cpp
	mortar.cpp
	movewith.cpp
	multiplay_gamerules.cpp
	nodes.cpp
	paranoia_wpn.cpp
	pathcorner.cpp
	plane.cpp
	plats.cpp
	player.cpp
	scripted.cpp
	singleplay_gamerules.cpp
	skill.cpp
	sound.cpp
	soundent.cpp
	spectator.cpp
	squadmonster.cpp
	subs.cpp
	Subtitle_Multiple.cpp
	talkmonster.cpp
	teamplay_gamerules.cpp
	triggers.cpp
	util.cpp
	weapons.cpp
	world.cpp
	../game_shared/voice_gamemgr.cpp
	../pm_shared/pm_debug.c
	../pm_shared/pm_math.c
	../pm_shared/pm_shared.c
	../public/safe_snprintf.c
)

if (HAVE_STRLCPY)
	add_definitions(-DHAVE_STRLCPY)
else()
	list(APPEND SVDLL_SOURCES
		../external/openbsd/strlcpy.c)
endif()

if (HAVE_STRLCAT)
	add_definitions(-DHAVE_STRLCAT)
else()
	list(APPEND SVDLL_SOURCES
		../external/openbsd/strlcat.c)
endif()

include_directories (. wpn_shared ../common ../engine ../pm_shared ../game_shared ../public)

if(MSVC)
	set(SVDLL_SOURCES
		${SVDLL_SOURCES}
		hl.def)
endif()

add_library (${SVDLL_LIBRARY} SHARED ${SVDLL_SOURCES})
target_link_libraries(${SVDLL_LIBRARY} vcs_info)

set_target_properties (${SVDLL_LIBRARY} PROPERTIES POSITION_INDEPENDENT_CODE 1)

if(APPLE AND NOT "${SERVER_LIBRARY_NAME_OSX}" STREQUAL "")
	# on Apple platforms we always use gamedll_osx liblist/gameinfo key, if it's set
	set(SVDLL_NAME "${SERVER_LIBRARY_NAME_OSX}")
elseif(NOT WIN32 AND NOT "${SERVER_LIBRARY_NAME_LINUX}" STREQUAL "")
	# on non-Windows platforms we use gamedll_linux liblist/gameinfo key, if it's set
	set(SVDLL_NAME "${SERVER_LIBRARY_NAME_LINUX}")
else()
	# on Windows or if no gamedll_osx or gamedll_linux is set, use default gamedll key
	set(SVDLL_NAME "${SERVER_LIBRARY_NAME}")
endif()

set_target_postfix_with_name(${SVDLL_LIBRARY} ${SVDLL_NAME})

if(HAVE_LTO)
	set_property(TARGET ${SVDLL_LIBRARY} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

if(XASH_PS4)
	# signed module and names of its functions, see cmake/PS4.cmake
	ps4_add_module(${SVDLL_LIBRARY} "${GAMEDIR}/${SERVER_INSTALL_DIR}/")
else()
	install(TARGETS ${SVDLL_LIBRARY}
		RUNTIME DESTINATION "${GAMEDIR}/${SERVER_INSTALL_DIR}/"
		LIBRARY DESTINATION "${GAMEDIR}/${SERVER_INSTALL_DIR}/"
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
			GROUP_READ GROUP_EXECUTE
			WORLD_READ WORLD_EXECUTE)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Release" AND NOT WIN32 AND NOT XASH_PS4)
	if(APPLE)
		# Apple strip treats -s as a symbol list file, -x keeps exported symbols
		set(STRIP_FLAGS -x)
	else()
		set(STRIP_FLAGS -s)
	endif()
	add_custom_command(TARGET ${SVDLL_LIBRARY}
			POST_BUILD DEPENDS ${SVDLL_LIBRARY}
			COMMAND ${CMAKE_STRIP} ${STRIP_FLAGS} $<TARGET_FILE:${SVDLL_LIBRARY}>)
endif()
