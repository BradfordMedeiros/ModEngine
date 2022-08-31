;;
;; Logic for different types of depth 

(define (getMockModelList)
	(list
		(list "AllMeshes" "mesh1" (list 0 0))
		(list "AllMeshes" "mesh2" (list 0 0))
	)
)

(format #t "list models: ~a\n" (ls-models))

(define (getMockScenegraph) (list
	(list "root" "mainfolder" (list 0 0))
	(list "root2" "mainfolder2" (list 0 1))
	(list "mainfolder" "folder1" (list 0 0))
	(list "mainfolder" "folder2" (list 0 0))
	(list "mainfolder" "folder3" (list 0 0))
	(list "folder3" "folder3-1" (list 0 0))
	(list "folder3" "folder3-2" (list 0 0))
))

(define (makeIntoModelGraph modelpath) (list "models" modelpath (list 0 0)))
(define (getModelList) (map makeIntoModelGraph (ls-models)))
(define (getNoData) (list (list "data" "none available" (list 0 0))))


(define (donothing x) #f)
(define (echoprint x) (format #t "x = ~a\n" x))
(define (selectScenegraphItem x) (format #t "placeholder select scenegraph item\n"))
(define (selectModelItem element) 
	(define modelpath (car element))
	(mk-obj-attr
		(string-append (number->string (random 10000000)) "-generated")
		(list
			(list "mesh" modelpath)
		)
	)
)

(define modeToGetDepGraph
	(list
		(list "nodata" getNoData donothing #f)
		(list "mock-scenegraph" getMockScenegraph donothing #f)
		(list "scenegraph" scenegraph selectScenegraphItem #t)
		(list "mock-models" getMockModelList donothing #f)
		(list "models" getModelList selectModelItem #f)
	)
)
(define (getDepGraph) #f)
(define showSceneIds #f)
(define handleItemSelected donothing)
(define (setDepGraphType type)
	(define depGraphPair (assoc type modeToGetDepGraph))
	(if depGraphPair
		(begin
			(set! getDepGraph (cadr depGraphPair))
			(set! handleItemSelected (caddr depGraphPair))
			(set! showSceneIds (cadddr depGraphPair))
		)
	)
)

(define (setTypeFromAttr)
	(define depgraphAttr (assoc "depgraph" (gameobj-attr mainobj)))
	(if depgraphAttr 
		(setDepGraphType (cadr depgraphAttr))
		(setDepGraphType "nodata")
	)
)
(setTypeFromAttr)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define fontsize 30)
(define (increaseFontSize)
	(set! fontsize (+ fontsize 1))
	(onGraphChange)
)
(define (decreaseFontSize)
	(set! fontsize (- fontsize 1))
	(onGraphChange)
)

(define texturename (string-append "gentexture-scenegraph" ))

(define (create-obj)
  (mk-obj-attr "someobj"     
  	(list
  		(list "position" (list 1 1 0))
  		(list "mesh" "./res/models/box/spriteplane.dae")
  	)
	)
  (gameobj-setattr! (lsobj-name "someobj/Plane") 
  	(list
  		(list "texture" texturename)
  	)
  )
)


(define depgraph (getDepGraph))
(define (refreshDepGraph) 
	(set! depgraph (getDepGraph))
)

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
(define maxOffset 0)

(define (calcSpacing) (* (/ fontsize 1000) 2))
(define (calcX depth) 
	(define spacingPerLetter (calcSpacing))
	(+ -1 (* 0.5 spacingPerLetter) (* depth spacingPerLetter))
)
(define (rawCalcY depth)
	(define spacingPerLetter (calcSpacing))
	(- 1 (* 0.5 spacingPerLetter) (* depth spacingPerLetter))
)
(define (calcY depth) (- (rawCalcY depth) offset))

(define (setMinOffset depth) 
	(define newMinOffset (rawCalcY depth))
	(if (< newMinOffset minOffset)
		(set! minOffset newMinOffset)
	)
)
(define (resetMinOffset) (set! minOffset 0))
(define minOffset (* -1 (rawCalcY 1)))

(define selectedIndex 1)
(define maxIndex #f)
(define selectedElement #f)
(define (setSelectedIndex index)
	(define adjustedIndex (max 0 index))
	(set! selectedIndex (if maxIndex (min maxIndex adjustedIndex) adjustedIndex))
	(onGraphChange)
)

(define (checkExpanded elementName sceneId)
	(define elementPath (expandPath elementName sceneId))
	(define value (assoc elementPath expandState))
	(if value (equal? #t (cadr value)) #t)
)


(define baseNumber 90000)  ; arbitrary number, only uses for mapping selection for now, which numbering is basically a manually process
(define (draw elementName sceneId depth height expanded)
	(define isSelected (equal? selectedIndex height))
	(if isSelected (set! selectedName (expandPath elementName sceneId)))
	(if isSelected (set! selectedElement (list elementName sceneId)))
	(draw-text-ndi
		(if showSceneIds (string-append elementName "(" (number->string sceneId) ")") elementName)
		(calcX depth) 
		(calcY height) 
		fontsize
		(if isSelected  (list 0.7 0.7 1 1) (list 1 1 1 1)) 
		textureId
		(+ baseNumber height)
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
(define (drawHierarchy target sceneId depth getIndex fullTarget)
	(define childElements (children target sceneId))
	(define isExpanded (checkExpanded target sceneId))
	(define exists (elementExists target))
	(if #t
		(begin
	    (draw target sceneId depth (getIndex) isExpanded)
	    (if isExpanded
	      (for-each 
	      	(lambda(target)
	      		(drawHierarchy (cadr target) (cadr (caddr target)) (+ depth 1) getIndex target)
	      	) 
	      	childElements
	      )
	    )
		)
	)
)

;	(list "mainfolder" "folder1" (list 0 0))
(define (fullParentName element) (list (car element) (car (caddr element))))
(define (fullChildName element) (list (cadr element)  (cadr (caddr element))))

(define (parentInList parentName parentSceneId children)
	(if (equal? (length children) 0)
		#f
		(let* ((currElement (car children)) (name (car currElement)) (sceneId (cadr currElement)))
			(if (and (equal? name parentName) (equal? sceneId parentSceneId))
				#t
				(parentInList parentName parentSceneId (cdr children))
			)
			
		)
	)
)

(define (make-set elementList) 
	(define uniqueList (list))
	(for-each (lambda(element) 
		(if (not (member element uniqueList))
			(set! uniqueList (cons element uniqueList))
		)
	) elementList)
	uniqueList
)

(define (allRootParents)
	(define allChildren (make-set (map fullChildName depgraph)))
	(define allParents (make-set (map fullParentName depgraph)))
	(define filteredParents (filter (lambda(parent) (not (parentInList (car parent) (cadr parent) allChildren))) allParents))
	;(format #t "all parents: ~a\n" allParents)
	;(format #t "all children: ~a\n" allChildren)
	;(format #t "filteredParents: ~a\n" filteredParents)
	filteredParents
)

(define drawtitle #f)
(define (addPermaData)
	(define index -1)
	(define getIndex (lambda()
		(set! index (+ index 1))
		(setMinOffset index)
		index
	))
	(if drawtitle (begin
		(draw-text "Scenegraph" 20 30 4 (list 1 1 1 0.8) textureId)
	  (draw-line (list -1 0.9 0) (list 1 0.9 0) (list 0 0 1 1) #t textureId)
	  (draw-line (list -1 0.9 0) (list 1 0.9 0) (list 0 0 1 1) #t textureId)
	  (draw-line (list -1 1 0)   (list 1 1 0)   (list 0 1 1 1) #f textureId)	
	))

;	(draw-line (list -1 0.9 0) (list 1 0.9 0)  #f textureId)
;	(draw-line (list -1 1 0)   (list 1 1 0)   #f textureId)

	(resetMinOffset)
	(for-each (lambda(target)
		(drawHierarchy (car target) (cadr target) 0 getIndex target)
	) (allRootParents))
	(set! maxIndex index)
)

(define (onGraphChange)
	(refreshDepGraph)         ; should this really be done at this point?  Perhaps?!
	(clear-texture textureId (list 0.1 0.1 0.1 1))
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
     	(if (equal? key 47) (create-obj))  ; /
     	(if (equal? key 264) (setSelectedIndex (+ selectedIndex 1)))                    
     	(if (equal? key 265) (setSelectedIndex (- selectedIndex 1))) 
     	(if (equal? key 257) (toggleExpanded))  ; enter
     	(if (equal? key 344) (handleItemSelected selectedElement))  ; shift
		)
	)
	(if (equal? key 61)
		(increaseFontSize)	; = 
	)
	(if (equal? key 45)
		(decreaseFontSize)  ; -
	)
)

(define (onScroll amount)
  (set! offset (min maxOffset (max minOffset (+ offset (* 0.04 amount)))))
  ;(format #t "minoffset: ~a, maxoffset: ~a, offset: ~a\n" minOffset maxOffset offset)
	(onGraphChange)
)