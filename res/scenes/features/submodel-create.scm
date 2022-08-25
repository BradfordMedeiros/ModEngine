(define madeobj #f)
(define (onKey key scancode action mods)
	(format #t "key is: ~a\n" key)
	(if (and (equal? action 1) (equal? key 257))
		(begin
			(format #t "making someobj2\n")
			(if (not madeobj)
				(begin
					(mk-obj-attr "someobj2"     
  					(list
  						(list "position" (list 0 2 0))
  					  (list "texture" "./res/textures/grid.png")
  					  (list "mesh" "../gameresources/build/debug/twospike.gltf")
  					  (list "someobj2/Cube.001" "tint" (list 0 1 0 1))
   					  (list "someobj2/Cube.001" "texture" "./res/textures/blacktop.jpg")
   					  (list "someobj2/Cube.001" "position" (list 0 -1 -1))
  					)
					)	
					(set! madeobj #t)
				)
			)	
		)
	)
)
