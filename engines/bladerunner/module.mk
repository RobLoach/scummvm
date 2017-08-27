MODULE := engines/bladerunner

MODULE_OBJS = \
	adq.o \
	aesc.o \
	actor.o \
	actor_clues.o \
	actor_combat.o \
	actor_walk.o \
	adpcm_decoder.o \
	ambient_sounds.o \
	archive.o \
	aud_stream.o \
	audio_mixer.o \
	audio_player.o \
	audio_speech.o \
	bladerunner.o \
	boundingbox.o \
	chapters.o \
	combat.o \
	crimes_database.o \
	decompress_lcw.o \
	decompress_lzo.o \
	detection.o \
	dialogue_menu.o \
	elevator.o \
	fog.o \
	font.o \
	gameflags.o \
	gameinfo.o \
	image.o \
	item.o \
	item_pickup.o \
	items.o \
	light.o \
	lights.o \
	matrix.o \
	mouse.o \
	movement_track.o \
	obstacles.o \
	outtake.o \
	overlays.o \
	regions.o \
	scene.o \
	scene_objects.o \
	script/script.o \
	script/init.o \
	script/kia.o \
	script/vk.o \
	script/esper.o \
	script/ai.o \
	script/ai/mccoy.o \
	script/ai/runciter.o \
	script/ai/officer_leary.o \
	script/ai/leon.o \
	script/scene.o \
	script/scene/ar01.o \
	script/scene/ar02.o \
	script/scene/bb01.o \
	script/scene/bb02.o \
	script/scene/bb03.o \
	script/scene/bb04.o \
	script/scene/bb05.o \
	script/scene/bb06.o \
	script/scene/bb07.o \
	script/scene/bb08.o \
	script/scene/bb09.o \
	script/scene/bb10.o \
	script/scene/bb11.o \
	script/scene/bb12.o \
	script/scene/bb51.o \
	script/scene/ct01.o \
	script/scene/ct02.o \
	script/scene/ct03.o \
	script/scene/ct04.o \
	script/scene/ct05.o \
	script/scene/ct06.o \
	script/scene/ct07.o \
	script/scene/ct08.o \
	script/scene/ct09.o \
	script/scene/ct10.o \
	script/scene/ct11.o \
	script/scene/ct12.o \
	script/scene/ct51.o \
	script/scene/dr01.o \
	script/scene/dr02.o \
	script/scene/dr03.o \
	script/scene/dr04.o \
	script/scene/dr05.o \
	script/scene/dr06.o \
	script/scene/hc01.o \
	script/scene/hc02.o \
	script/scene/hc03.o \
	script/scene/hc04.o \
	script/scene/hf01.o \
	script/scene/hf02.o \
	script/scene/hf03.o \
	script/scene/hf04.o \
	script/scene/hf05.o \
	script/scene/hf06.o \
	script/scene/hf07.o \
	script/scene/kp01.o \
	script/scene/kp02.o \
	script/scene/kp03.o \
	script/scene/kp04.o \
	script/scene/kp05.o \
	script/scene/kp06.o \
	script/scene/kp07.o \
	script/scene/ma01.o \
	script/scene/ma02.o \
	script/scene/ma04.o \
	script/scene/ma05.o \
	script/scene/ma06.o \
	script/scene/ma07.o \
	script/scene/ma08.o \
	script/scene/nr01.o \
	script/scene/nr02.o \
	script/scene/nr03.o \
	script/scene/nr04.o \
	script/scene/nr05.o \
	script/scene/nr06.o \
	script/scene/nr07.o \
	script/scene/nr08.o \
	script/scene/nr09.o \
	script/scene/nr10.o \
	script/scene/nr11.o \
	script/scene/ps01.o \
	script/scene/ps02.o \
	script/scene/ps03.o \
	script/scene/ps04.o \
	script/scene/ps05.o \
	script/scene/ps06.o \
	script/scene/ps07.o \
	script/scene/ps09.o \
	script/scene/ps10.o \
	script/scene/ps11.o \
	script/scene/ps12.o \
	script/scene/ps13.o \
	script/scene/ps14.o \
	script/scene/ps15.o \
	script/scene/rc01.o \
	script/scene/rc02.o \
	script/scene/rc03.o \
	script/scene/rc04.o \
	script/scene/rc51.o \
	script/scene/tb02.o \
	script/scene/tb03.o \
	script/scene/tb05.o \
	script/scene/tb06.o \
	script/scene/tb07.o \
	script/scene/ug01.o \
	script/scene/ug02.o \
	script/scene/ug03.o \
	script/scene/ug04.o \
	script/scene/ug05.o \
	script/scene/ug06.o \
	script/scene/ug07.o \
	script/scene/ug08.o \
	script/scene/ug09.o \
	script/scene/ug10.o \
	script/scene/ug12.o \
	script/scene/ug13.o \
	script/scene/ug14.o \
	script/scene/ug15.o \
	script/scene/ug16.o \
	script/scene/ug17.o \
	script/scene/ug18.o \
	script/scene/ug19.o \
	set.o \
	settings.o \
	set_effects.o \
	shape.o \
	slice_animations.o \
	slice_renderer.o \
	spinner.o \
	suspects_database.o \
	text_resource.o \
	ui_image_picker.o \
	view.o \
	vqa_decoder.o \
	vqa_player.o \
	waypoints.o \
	zbuffer.o

# This module can be built as a plugin
ifeq ($(ENABLE_BLADERUNNER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
