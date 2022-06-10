(define texturename "graphs-testplot")

(define (create-obj)
	(format #t "create obj placeholder\n")
   	(mk-obj-attr "someobj"     
  		(list
  			(list "position" (list 1 1 0))
  			(list "mesh" "./res/models/box/spriteplane.dae")
  			(list "rotation" (list 0 0 1 180))
  		)
	)
  (gameobj-setattr! (lsobj-name "someobj/Plane") 
  	(list
  		(list "texture" texturename)
  	)
  )
)

(define depgraph (list
	(list "human" "leftarm")
	(list "human" "rightarm")
	(list "rightarm" "righthand")
	(list "leftarm" "lefthand")
	(list "human" "head")
	(list "human" "legs")
))


(define offset 0)
(define (calcX depth) (+ (* 20 depth) 20))
(define (calcY depth) (+ (* 20 depth) 80 offset))

(define (selected index name) (if (equal? index 2) (string-append ">" name) name))

(define (draw elementName depth height)
	(define childElements (map cadr (filter (lambda(val) (equal? (car val) "human")) depgraph)))
	(define isExpanded #t)

	(draw-text "human" (calcX depth) (calcY height) 4 #f textureId)
	(if isExpanded
		(format #t "draw child elements here\n")
	)

)
(define (addPermaData)
	(draw-text "Scenegraph" 20 30 4 #f textureId)
	(draw-line (list -1 0.9 0) (list 1 0.9 0) #f textureId)
	(draw-line (list -1 1 0) (list 1 1 0) #f textureId)

	(draw "human" 0 0)

	;(draw-text "leftarm" (calcX 1) (calcY 1) 4 #f textureId)
	;(draw-text "left-hand" (calcX 2) (calcY 2) 4 #f textureId)
	;(draw-text "left-elbow" (calcX 2) (calcY 3) 4 #f textureId)

)

(define textureId #f)
(define texturename texturename)
(set! textureId (create-texture texturename 1000 1000))
(addPermaData)


(define (onKeyChar codepoint)
	(if (equal? codepoint 47) (create-obj))                        
)

(define (onScroll amount)
  (set! offset (+ offset (* 10 amount)))
	(clear-texture textureId)
  (addPermaData)
)