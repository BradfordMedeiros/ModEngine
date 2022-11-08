main: resources
	@php ./editor.rawscene.php main > ./build/editor.rawscene
	@php ./dock/main.php main object_details > ./build/dock/editor_object_details.rawscene
	@php ./dock/main.php main scene_info > ./build/dock/editor_scene_info.rawscene
	@php ./dock/main.php main object_tools > ./build/dock/editor_object_tools.rawscene
	@php ./dock/main.php main transform > ./build/dock/editor_transform.rawscene
	@php ./dock/main.php main scenegraph > ./build/dock/editor_scenegraph.rawscene
	@php ./dock/main.php main performance > ./build/dock/editor_performance.rawscene
	@php ./dock/main.php main cameras > ./build/dock/editor_cameras.rawscene
	@php ./dock/main.php main lights > ./build/dock/editor_lights.rawscene
	@php ./dock/main.php main sound > ./build/dock/editor_sound.rawscene
	@php ./dock/main.php main text > ./build/dock/editor_text.rawscene
	@php ./dock/main.php main world > ./build/dock/editor_world.rawscene
	@php ./dock/main.php main rendering > ./build/dock/editor_rendering.rawscene
	@php ./dock/main.php main models > ./build/dock/editor_models.rawscene
	@php ./dock/main.php main textures > ./build/dock/editor_textures.rawscene
	@php ./dock/main.php main not_implemented > ./build/dock/editor_not_implemented.rawscene

resources: build
	@mkdir -p ./build/dock
	@cp -r ./dock/images ./build/dock	
gameplay: resources
	@php ./editor.rawscene.php gameplay > ./build/editor.rawscene
	@php ./dock/main.php gameplay movement  > ./build/dock/editor_movement.rawscene
	@php ./dock/main.php gameplay weapons  > ./build/dock/editor_weapons.rawscene

build:
	@mkdir -p ./build
clean:
	@rm -rf ./build
