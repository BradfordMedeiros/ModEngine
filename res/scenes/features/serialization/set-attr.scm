
(define target (cadr (assoc "target" (gameobj-attr mainobj))))
(define targetObj (lsobj-name target))

(define typeToAttr (list
	(list ">camera" 
		(list
			(list "dof" "disabled")
			(list "target" "wow")
			(list "maxblur" 8)
			(list "minblur" 2)
			(list "bluramount" 16)
		)
	)
	(list "!light" 
		(list
			(list "color" (list 1 0 0))
			;(list "type" "directional")
			;(list "attenuation" (list 4 3 1))
			(list "position" (list 0 2 0))
			(list "scale" (list 2 1 1))
			;(list "lookat" "box")
			(list "physics" "enabled")
			(list "physics_shape" "shape_sphere")
			(list "customattr" "overrideenvalue")
		)
	)
	(list "*button" 
		(list
			(list "state" "on")
			(list "tint" (list 0 0 1 1))
			(list "ontint" (list 0 1 0 1))
			(list "ontexture" "./res/textures/blacktop.jpg")
			(list "offtexture" "./res/textures/wood.jpg")
		)
	)
	(list "&sample"
		(list
			(list "loop" "true")
			(list "clip" "./res/sounds/silenced-gunshot.wav")
		)
	)
	(list "_slider"
		(list
  		(list "min" 20)
  		(list "max" 100)
  		(list "slideamount" 0.3)
		)
	)
	(list "(layout"
		(list
  		(list "tint" (list 1 0 0 0.5))
  		(list "border-size" 0.5)
  		(list "texture" "./res/textures/blacktop.jpg")
		)
	)
	(list ")text"
		(list
			(list "value" "goodbye world")
			(list "tint" (list 0 1 0 1))
			(list "align" "right")
			(list "wraptype" "char")
			(list "wrapamount" 3)
			(list "charlimit" 9)
			(list "cursor" 2)
			(list "cursor-dir" "right")
			(list "cursor-highlight" 3)
			(list "font" "./res/fonts/dpquake.ttf")
			(list "maxheight" 2)
		)
	)
	(list "mesh/Plane_Plane.001" 
		(list
			(list "texture" "./res/textures/wood.jpg")
			(list "tint" (list 1 0 1 0.8))
			;(list "disabled" "true")
		)
	)
	(list "@portal"
		(list
			(list "perspective" "false")
		)
	)
	(list "<line"
		(list
			(list "points" "0 0 0|5 0 0|5 0 -5")
		)
	)
	(list "-heightmap"
		(list
			(list "texture" "./res/textures/grass.jpg")
			(list "texturetiling" "100 10")
			(list "fragshader" "./res/shaders/discard_lowintensity/fragment.glsl")
		)
	)
	
))

(define (attrForName)
	(cadr (assoc target typeToAttr))
)	

(define (onKey key scancode action mods)
	(if (and (equal? action 1) (equal? key 344)) ; shift
		(begin
			(format #t "set attr, target is: ~a\n" targetObj)
			(format #t "attr is: ~a" (attrForName))
			(gameobj-setattr! targetObj (attrForName))
		)
	)

	(if (and (equal? action 1) (equal? key 259)) ; shift
		(if (equal? target "&sample")
			(playclip "&sample")
		)
	)

	(format #t "key is: ~a\n" key)
)