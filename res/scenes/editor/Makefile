main:
	@php ./editor.rawscene.php main > ./editor.rawscene
	@php ./dock/main.php main object_details > ./dock/editor_object_details.rawscene
	@php ./dock/main.php main scene_info > ./dock/editor_scene_info.rawscene
	@php ./dock/main.php main object_tools > ./dock/editor_object_tools.rawscene
	@php ./dock/main.php main transform > ./dock/editor_transform.rawscene
	@php ./dock/main.php main scenegraph > ./dock/editor_scenegraph.rawscene
	@php ./dock/main.php main performance > ./dock/editor_performance.rawscene
	@php ./dock/main.php main cameras > ./dock/editor_cameras.rawscene
	@php ./dock/main.php main lights > ./dock/editor_lights.rawscene
	@php ./dock/main.php main sound > ./dock/editor_sound.rawscene
	@php ./dock/main.php main text > ./dock/editor_text.rawscene
	@php ./dock/main.php main world > ./dock/editor_world.rawscene
	@php ./dock/main.php main rendering > ./dock/editor_rendering.rawscene
	@php ./dock/main.php main models > ./dock/editor_models.rawscene
	@php ./dock/main.php main textures > ./dock/editor_textures.rawscene
	@php ./dock/main.php main not_implemented > ./dock/editor_not_implemented.rawscene

gameplay:
	@php ./editor.rawscene.php gameplay > ./editor.rawscene
	@php ./dock/main.php gameplay movement  > ./dock/editor_movement.rawscene
	@php ./dock/main.php gameplay weapons  > ./dock/editor_weapons.rawscene

clean:
	@rm ./dock/editor*.rawscene
