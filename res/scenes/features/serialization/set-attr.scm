
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
			(list "type" "directional")
			(list "attenuation" (list 4 3 1))
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
)