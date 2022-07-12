(define texturename "generated-texture")
(define createdTex #f)
(define (createTex)
  (define textureId (create-texture texturename 1000 1000))
  (draw-line (list -1 1 0) (list 1 -1 0) #f textureId)
  (draw-line (list 1 1 0) (list -1 -1 0)  #f textureId)
  (format #t "created texture: ~a\n" texturename)
  (set! createdTex #t)
  ;(gameobj-setattr! (lsobj-name "1/Plane") 
  ;	(list
  ;		(list "texture" texturename)
  ;	)
	;)
)

(define (onFrame)
	(if (and (> (time-seconds) 3) (not createdTex))
		(createTex)
	)
)



