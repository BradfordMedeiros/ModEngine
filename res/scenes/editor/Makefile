
main: resources
	@php ./editor.rawscene.php config/main/editor_config.php > ./build/editor.rawscene
	@php ./dock/main.php config/main/dock_config.php object_details > ./build/dock/editor_object_details.rawscene
	@php ./dock/main.php config/main/dock_config.php scene_info > ./build/dock/editor_scene_info.rawscene
	@php ./dock/main.php config/main/dock_config.php object_tools > ./build/dock/editor_object_tools.rawscene
	@php ./dock/main.php config/main/dock_config.php transform > ./build/dock/editor_transform.rawscene
	@php ./dock/main.php config/main/dock_config.php scenegraph > ./build/dock/editor_scenegraph.rawscene
	@php ./dock/main.php config/main/dock_config.php performance > ./build/dock/editor_performance.rawscene
	@php ./dock/main.php config/main/dock_config.php cameras > ./build/dock/editor_cameras.rawscene
	@php ./dock/main.php config/main/dock_config.php lights > ./build/dock/editor_lights.rawscene
	@php ./dock/main.php config/main/dock_config.php sound > ./build/dock/editor_sound.rawscene
	@php ./dock/main.php config/main/dock_config.php text > ./build/dock/editor_text.rawscene
	@php ./dock/main.php config/main/dock_config.php world > ./build/dock/editor_world.rawscene
	@php ./dock/main.php config/main/dock_config.php rendering > ./build/dock/editor_rendering.rawscene
	@php ./dock/main.php config/main/dock_config.php models > ./build/dock/editor_models.rawscene
	@php ./dock/main.php config/main/dock_config.php textures > ./build/dock/editor_textures.rawscene
	@php ./dock/main.php config/main/dock_config.php not_implemented > ./build/dock/editor_not_implemented.rawscene

gameplay: resources
	@php ./editor.rawscene.php config/gameplay/editor_config.php > ./build/editor.rawscene
	@php ./dock/main.php config/gameplay/dock_config.php movement  > ./build/dock/editor_movement.rawscene
	@php ./dock/main.php config/gameplay/dock_config.php weapons  > ./build/dock/editor_weapons.rawscene

resources: build
	@mkdir -p ./build/dock
	@cp -r ./dock/images ./build/dock	

build:
	@mkdir -p ./build
clean:
	@rm -rf ./build
