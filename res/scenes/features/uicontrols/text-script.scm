
(define (varyingValue)
	(define time (time-seconds))
	(define value (inexact->exact (round time)))
	(define remainder (modulo value 4))
	(/ remainder 4)
)	

(define fontsize 18)
(define (fontspace index) (+ 1 (* -1 (+ 1 index) fontsize 0.01)))
(define (onFrame)
	(draw-text-ndi "red text" -0.5 (fontspace 0) fontsize (list 1 0 0 1))
	(draw-text-ndi "green text" -0.5 (fontspace 1) fontsize (list 0 1 0 1))
	(draw-text-ndi "varying text" -0.5 (fontspace 2) fontsize (list (varyingValue) (varyingValue) 1 1))
	(draw-text-ndi "small text" -0.5 (fontspace 3) (inexact->exact (floor (* 0.5 fontsize))) (list 1 1 1 1))
	(draw-text-ndi "transparent" -0.5 (fontspace 4) fontsize (list 1 1 1 0.3))

	(draw-text "non-ndi text" 100 100 fontsize (list 1 1 1 1))
)

(draw-text-ndi "perma text" -0.5 (fontspace 5) fontsize (list 1 1 1 1) #t)
(draw-text "non-ndi perma" 100 200 fontsize (list 1 1 1 1) #t)


(define textureId #f)
(define texturename "gentex-testtexture")
(set! textureId (create-texture texturename 1000 1000))
(define (create-obj)
	(format #t "create obj placeholder\n")
   	(mk-obj-attr "someobj"     
  		(list
  			(list "position" (list 1 0 -4))
  			(list "mesh" "./res/models/box/spriteplane.dae")
  		)
	)
  (gameobj-setattr! (lsobj-name "someobj/Plane") 
  	(list
  		(list "texture" texturename)
  	)
  )
)

(create-obj)
(clear-texture textureId (list 0 0 1 0.8))

(draw-text-ndi "Text on Texture" 0 0 20 (list 1 1 1 1) #f textureId)
(draw-text "Non-ndi text on Texture" 100 100 20 (list 1 1 1 1) #f textureId)

