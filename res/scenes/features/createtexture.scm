
(define (create-obj)
	(format #t "create obj placeholder\n")
   	(mk-obj-attr "someobj"     
  		(list
  			(list "position" (list 1 1 0))
  			(list "mesh" "./res/models/box/spriteplane.dae")
  		)
	)
    (format #t "value is: ~a\n" (lsobj-name "someobj/Plane"))
    (gameobj-setattr! (lsobj-name "someobj/Plane") 
    	(list
    		(list "texture" "graphs-testplot")
    	)
    )

)



; 1/Plane:texture:graphs-testplot

(define texturename "graphs-testplot")
(define (onKeyChar codepoint)
	(format #t "key char: ~a\n" codepoint)
	(if (equal? codepoint 44) (create-texture texturename 1000 1000)) ; ,
	(if (equal? codepoint 46) (free-texture texturename))           ; . 
	(if (equal? codepoint 47) (create-obj))                         ; / 
)

