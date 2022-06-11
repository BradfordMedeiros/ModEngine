
(define texturename (string-append "texture-" (number->string (gameobj-id mainobj))))

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


(define depgraph (scenegraph))
(format #t "dep graph:\n~a\n" depgraph)

(define expandState (list))

(define selectedName #f)
(define (toggleExpanded)
	(define element (assoc selectedName expandState))
	(define isExpanded (if element (cadr element) #t))
	(set! expandState (cons  (list selectedName (not isExpanded)) expandState))
	(onGraphChange)
)

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
(define (selected name expanded index) 
	(define isSelected (equal? selectedIndex index))
	(define selectedPrefix (if isSelected "-" " "))
	(if isSelected (set! selectedName name))
	(if expanded (string-append selectedPrefix ">" name) (string-append selectedPrefix "^" name))
)

(define (checkExpanded elementName)
	(define value (assoc elementName expandState))
	(if value (equal? #t (cadr value)) #t)
)
(define (draw elementName depth height expanded)
	(draw-text (selected elementName expanded height) (calcX depth) (calcY height) 4 (list 0 1 0 1) textureId)
)

(define (elementExists element)
	(define value (assoc element depgraph))
	(if value #t (if (member element depgraph) #t #f))  ; check if 

)
(define (drawHierachy target depth getIndex)
	(define childElements (map cadr (filter (lambda(val) (equal? (car val) target)) depgraph)))
	(define isExpanded (checkExpanded target))
	(define exists (elementExists target))
	(if #t
		(begin
	    (draw target depth (getIndex) isExpanded)
	    (if isExpanded
	      (for-each 
	      	(lambda(target)
	      		(format #t "draw target: ~a\n" target)
	      		(drawHierachy target (+ depth 1) getIndex)
	      	) 
	      	childElements
	      )
	    )
		)
	)
)

(define (addPermaData)
	(define index -1)
	(define getIndex (lambda()
		(set! index (+ index 1))
		index
	))
	(draw-text "Scenegraph" 20 30 4 (list 1 1 1 0.8) textureId)
	(draw-line (list -1 0.9 0) (list 1 0.9 0) #f textureId)
	(draw-line (list -1 1 0) (list 1 1 0) #f textureId)
	(drawHierachy "body" 0 getIndex)
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
	(format #t "key is: ~a\n" key)
	(if (equal? action 1)
		(begin
     	(if (equal? key 47) (create-obj))
     	(if (equal? key 264) (setSelectedIndex (+ selectedIndex 1)))                    
     	(if (equal? key 265) (setSelectedIndex (- selectedIndex 1)))
     	(if (equal? key 257) (toggleExpanded))
		)
	)
)

(define (onScroll amount)
  (set! offset (+ offset (* 10 amount)))
	(onGraphChange)
)