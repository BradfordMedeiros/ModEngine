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

(define selectedIndex 1)
(define maxIndex #f)
(define (setSelectedIndex index)
	(define adjustedIndex (max 0 index))
	(set! selectedIndex (if maxIndex (min maxIndex adjustedIndex) adjustedIndex))
	(onGraphChange)
)
(define (selected name index) (if (equal? selectedIndex index) (string-append ">" name) name))

(define (draw elementName depth height)
	(define childElements (map cadr (filter (lambda(val) (equal? (car val) elementName)) depgraph)))
	(define isExpanded #t)
	(draw-text (selected elementName height) (calcX depth) (calcY height) 4 (list 0 1 0 1) textureId)
	(if isExpanded
		(format #t "draw child elements here\n")
	)
)

(define (addPermaData)
	(draw-text "Scenegraph" 20 30 4 (list 1 1 1 0.8) textureId)
	(draw-line (list -1 0.9 0) (list 1 0.9 0) #f textureId)
	(draw-line (list -1 1 0) (list 1 1 0) #f textureId)

	(draw "human" 0 0)
	(draw "human" 0 1)
	(draw "human" 0 2)
)

(define (onGraphChange)
	(clear-texture textureId (list 1 1 1 0.2))
	(addPermaData)
)

(define textureId #f)
(define texturename texturename)
(set! textureId (create-texture texturename 1000 1000))
(onGraphChange)

(define (onKey key scancode action mods)
	(if (equal? action 1)
		(begin
     	(if (equal? key 47) (create-obj))
     	(if (equal? key 264) (setSelectedIndex (+ selectedIndex 1)))                    
     	(if (equal? key 265) (setSelectedIndex (- selectedIndex 1)))
		)
	)
	(format #t "selected index is: ~a\n" selectedIndex)

)

(define (onScroll amount)
  (set! offset (+ offset (* 10 amount)))
	(onGraphChange)
)