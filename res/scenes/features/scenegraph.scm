
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


(define (getscenegraph) (scenegraph))

; parent, child, parent scene, child scene
;(define (getscenegraph) (list
;	(list "root"  "human"      (list 0 0))
;	(list "human" "head"       (list 0 0))
;	(list "human" "legs"       (list 0 0))
;	(list "human" "right-leg"  (list 0 1))
;	(list "legs"  "left-left"  (list 0 0))
;	(list "legs"  "right-leg"  (list 0 0))
;	(list "right-leg"  "right-toe"  (list 1 1))
;	(list "human" "arms"       (list 0 0))
;))

(define depgraph (getscenegraph))
(define (refreshDepGraph) (set! depgraph (getscenegraph)))

(define expandState (list))
(define (expandPath name sceneId) (string-append name ":" (number->string sceneId)))

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
	(if expanded (string-append selectedPrefix ">" name) (string-append selectedPrefix "^" name))
)

(define (checkExpanded elementName sceneId)
	(define elementPath (expandPath elementName sceneId))
	(define value (assoc elementPath expandState))
	(if value (equal? #t (cadr value)) #t)
)
(define (draw elementName sceneId depth height expanded)
	(define isSelected (equal? selectedIndex height))
	(if isSelected (set! selectedName (expandPath elementName sceneId)))
	(draw-text 
		(selected (string-append elementName "(" (number->string sceneId) ")") expanded height) 
		(calcX depth) 
		(calcY height) 
		(if isSelected 5 4) 
		(if isSelected  (list 0.7 0.7 1 1) (list 1 1 1 1)) 
		textureId
	)
)

(define (elementExists element)
	(define value (assoc element depgraph))
	(if value #t (if (member element depgraph) #t #f))  ; check if 

)

(define (children target sceneId)
		(filter 
			(lambda(val)
				(and 
					(equal? (car val) target) 
					(equal? (car (caddr val)) sceneId)
				)
			) 
			depgraph
		)
	
)
(define (drawHierarchy target sceneId depth getIndex)
	(define childElements (children target sceneId))
	(define isExpanded (checkExpanded target sceneId))
	(define exists (elementExists target))
	(if #t
		(begin
	    (draw target sceneId depth (getIndex) isExpanded)
	    (if isExpanded
	      (for-each 
	      	(lambda(target)
	      		(format #t "draw target: ~a\n" target)
	      		(drawHierarchy (cadr target) (cadr (caddr target)) (+ depth 1) getIndex)
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
	(draw-line (list -1 0.9 0) (list 1 0.9 0) (list 0 0 1 1) #t textureId)
	(draw-line (list -1 0.9 0) (list 1 0.9 0) (list 0 0 1 1) #t textureId)
	(draw-line (list -1 1 0)   (list 1 1 0)   (list 0 1 1 1) #f textureId)
;	(draw-line (list -1 0.9 0) (list 1 0.9 0)  #f textureId)
;	(draw-line (list -1 1 0)   (list 1 1 0)   #f textureId)

	(drawHierarchy "root" 0 0 getIndex)
	(set! maxIndex index)
)

(define (onGraphChange)
	(refreshDepGraph)         ; should this really be done at this point?  Perhaps?!
	(clear-texture textureId (list 0.1 0.1 0.1 0.8))
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