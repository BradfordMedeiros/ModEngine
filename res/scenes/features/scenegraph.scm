;;
;; Logic for different types of depth 
(use-modules (srfi srfi-1))  ; for list-index


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


(define (donothing x isAlt) #f)
(define (echoprint x) (format #t "x = ~a\n" x))
(define (selectScenegraphItem x isAlt) 
  (define objname (car x))
  (define sceneId (cadr x))
  (define selectedObj (lsobj-name objname sceneId))
  (define objIndex (gameobj-id selectedObj))
  (define objpos (gameobj-pos-world selectedObj))
  (if isAlt
  	(mov-cam (car objpos) (cadr objpos) (caddr objpos) #f)
  	(set-wstate (list
  		(list "editor" "selected-index" (number->string objIndex))
  	))
  )

)
(define (selectModelItem element isAlt) 
	(define modelpath (car element))
	(mk-obj-attr
		(string-append (number->string (random 10000000)) "-generated")
		(list
			(list "mesh" modelpath)
		)
	)
)

(define (onObjDoNothing gameobj color) 
	(format #t "on obj do nothing\n")
	#f
)
(define (onObjectSelectedSelectItem gameobj color)
	(refreshDepGraph)
	(let (
			(index (selectedIndexForGameobj gameobj))
		)
		(if index
			(begin
		    (format #t "on object selected: ~a, ~a\n" gameobj color)
		    (setSelectedIndex index)
		    (refreshGraphData)   
		    (onGraphChange)
			)
		)
	)
)

(define modeToGetDepGraph
	(list
		(list "nodata" getNoData donothing #f onObjDoNothing)
		(list "mock-scenegraph" getMockScenegraph donothing #f onObjDoNothing)
		(list "scenegraph" scenegraph selectScenegraphItem #t onObjectSelectedSelectItem)
		(list "mock-models" getMockModelList donothing #f onObjDoNothing)
		(list "models" getModelList selectModelItem #f onObjDoNothing)
	)
)
(define (getDepGraph) #f)
(define showSceneIds #f)
(define handleItemSelected donothing)
(define onObjectSelected onObjDoNothing)

(define (setDepGraphType type)
	(define depGraphPair (assoc type modeToGetDepGraph))
	(if depGraphPair
		(begin
			(set! getDepGraph (cadr depGraphPair))
			(set! handleItemSelected (caddr depGraphPair))
			(set! onObjectSelected (list-ref depGraphPair 4))
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
	(refreshDepGraph)         
	(onGraphChange)
)
(define (decreaseFontSize)
	(set! fontsize (- fontsize 1))
	(refreshDepGraph)   
	(onGraphChange)
)

(define texturename (string-append "gentexture-scenegraph" ))

(define (create-obj)
  (mk-obj-attr "someobj"     
  	(list
  		(list "position" (list 1 1 0))
  		(list "mesh" "./res/models/box/spriteplane.dae")
  		(list "someobj/Plane" "texture" texturename)
  		(list "layer" "noselect1")
  	)
	)
)


(define depgraph (getDepGraph))
(define (refreshDepGraph) 
	(set! depgraph (getDepGraph))
)

(define expandState (list))
(define (expandPath name sceneId) (string-append name ":" (number->string sceneId)))

(define (rmFromList listVals keyToRemove) 
	(filter 
		(lambda(val) 
			(format #t "key = ~a, val = ~a\n" keyToRemove val)
			(not (equal? (car val) keyToRemove))
		) 
		listVals
	)
)
(define (toggleExpanded name)
	(define element (assoc name expandState))
	(define isExpanded (if element (cadr element) #t))
	(define filteredList (rmFromList expandState name))
	(define newExpandState (cons  (list name (not isExpanded)) filteredList))
	(format #t "toggle expanded, selectname = ~a, element = ~a\n" name element)
	(set! expandState newExpandState)
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
(define selectedName #f)

; should set selectedElement and selectedName here
(define (setSelectedIndex index)
	(define adjustedIndex (max 0 index))
	(format #t "selected index: ~a\n" index)
	(set! selectedIndex (if maxIndex (min maxIndex adjustedIndex) adjustedIndex))
)

(define (checkExpanded elementName sceneId)
	(define elementPath (expandPath elementName sceneId))
	(define value (assoc elementPath expandState))
	(if value (equal? #t (cadr value)) #t)
)



(define baseNumber 90000)  ; arbitrary number, only uses for mapping selection for now, which numbering is basically a manually process
(define baseNumberMapping (list))
(define (baseNumberToSelectedIndex mappingIndex)
	(define isPayloadMapping (>= mappingIndex 95000))
	(define index (if isPayloadMapping (- mappingIndex 5000) mappingIndex))
	(define baseIndexPair (assoc index baseNumberMapping))
	(format #t "base mapping: ~a\n" baseNumberMapping)
	(format #t "base index pair: ~a\n" baseIndexPair)
	(if baseIndexPair
		(list (cadr baseIndexPair) isPayloadMapping)
		(begin
			(format #t "warning: no basemapping for: ~a\n" index)
			#f
		)
	)
)
(define (payloadMappingNumber mappingNumber) (+ mappingNumber 5000)
)
(define (clearBaseNumberMapping) (set! baseNumberMapping (list)))
(define (setBaseNumberMapping basenumber index)
	(set! baseNumberMapping (cons (list basenumber index) baseNumberMapping))
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

(define (calcDrawHierarchy target sceneId depth getIndex addDrawList)
	(define childElements (children target sceneId))
	(define isExpanded (checkExpanded target sceneId))
	(define height (getIndex))
	(define mappingNumber (+ baseNumber height))
	(begin
		 (addDrawList target sceneId depth height isExpanded mappingNumber)
	   (if isExpanded
	     (for-each 
	     	(lambda(target)
	     		(calcDrawHierarchy (cadr target) (cadr (caddr target)) (+ depth 1) getIndex addDrawList)
	     	) 
	     	childElements
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

(define (getDrawList)
	(define index -1)
	(define getIndex (lambda()
		(set! index (+ index 1))
		index
	))

	(define drawList (list))
	(define addDrawList (
		lambda(target sceneId depth height isExpanded mappingNumber)
			(set! drawList (reverse (cons (list target sceneId depth height isExpanded (equal? selectedIndex height) mappingNumber) (reverse drawList))))
			;(format #t "add draw list: ~a\n" drawList)
		)
	)
	(for-each 
		(lambda(target)
			(begin
				(calcDrawHierarchy (car target) (cadr target) 0 getIndex addDrawList)
			)
		) 
		(allRootParents)
	)
	(list drawList index)
)
(define (updateDrawListData drawList index)
	(resetMinOffset)
	(clearBaseNumberMapping)
	(set! maxIndex index)
	(setMinOffset index)
	(for-each
		(lambda (drawElement)
			(let (
					(isSelected (list-ref drawElement 5))
					(target (list-ref drawElement 0))
					(sceneId (list-ref drawElement 1))
					(mappingNumber (list-ref drawElement 6))
					(height (list-ref drawElement 3))
				)
				(if isSelected (set! selectedName (expandPath target sceneId)))
				(if isSelected (set! selectedElement (list target sceneId)))
				(setBaseNumberMapping mappingNumber height)
			)
		)
		drawList
	)
)
(define (doDrawList drawList)
	(for-each
		(lambda (drawElement)
			(let (
					(elementName (list-ref drawElement 0))
					(sceneId (list-ref drawElement 1))
					(depth (list-ref drawElement 2))
					(height (list-ref drawElement 3))
					(expanded (list-ref drawElement 4))
					(isSelected (list-ref drawElement 5))
					(mappingNumber (list-ref drawElement 6))
				)
				(draw-text-ndi
					(if expanded "E" "N")
					(calcX depth) 
					(calcY height) 
					fontsize
					;"./res/fonts/Walby-Regular.ttf"
					"./res/textures/fonts/gamefont"
					(if isSelected  (list 0.7 0.7 1 1) (list 1 1 1 1)) 
					textureId
					mappingNumber
				)
				(draw-text-ndi
					(if showSceneIds (string-append elementName "(" (number->string sceneId) ")") elementName)
					(calcX (+ depth 1)) 
					(calcY height) 
					fontsize
					;"./res/fonts/Walby-Regular.ttf"
					"./res/textures/fonts/gamefont"
					(if isSelected  (list 0.7 0.7 1 1) (list 1 1 1 1)) 
					textureId
					(payloadMappingNumber mappingNumber)
				)
			)
			
		)
		drawList
	)
)


(define (refreshGraphData)
	(let* (
			(drawListWithIndex (getDrawList))
			(drawList (car drawListWithIndex))
			(index (cadr drawListWithIndex))
		)
		(updateDrawListData drawList index)
  	drawList
	)
)

(define drawtitle #f)
(define (onGraphChange)
	(clear-texture textureId (list 0.1 0.1 0.1 1))
	(if drawtitle 
		(begin
			(draw-text "Scenegraph" 20 30 4 (list 1 1 1 0.8) textureId)
	  	(draw-line (list -1 0.9 0) (list 1 0.9 0) (list 0 0 1 1) #t textureId)
	  	(draw-line (list -1 0.9 0) (list 1 0.9 0) (list 0 0 1 1) #t textureId)
	  	(draw-line (list -1 1 0)   (list 1 1 0)   (list 0 1 1 1) #f textureId)	
		)
	)
	(doDrawList (refreshGraphData))
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
     	(if (equal? key 264) 
     		(begin
     			(setSelectedIndex (+ selectedIndex 1))
     			(refreshDepGraph)   
     			(onGraphChange)
     		)
     	)                  
     	(if (equal? key 265) 
     		(begin
     			(setSelectedIndex (- selectedIndex 1))
     			(refreshDepGraph)   
     			(onGraphChange)
     		)	
     	)
     	(if (equal? key 257) 
     		(begin
     			(toggleExpanded selectedName)
     			(refreshDepGraph)   
   				(onGraphChange)
     		)  ; enter
     	)
     	(if (equal? key 344) (handleItemSelected selectedElement #f))  ; shift
     	(if (equal? key 345) (handleItemSelected selectedElement #t))  ; ctrl
		)
	)
	(if (equal? key 61)
		(increaseFontSize)	; = 
	)
	(if (equal? key 45)
		(decreaseFontSize)  ; -
	)
	;(if (equal? key 262)
	;	(format #t "base: ~a\n" baseNumberMapping)
	;)
	;(format #t "selected_index = ~a, selected_name = ~a, selected_element = ~a\n" selectedIndex selectedName selectedElement)
)

(define (onScroll amount)
	(format #t "onScroll called: ~a" amount)
  (set! offset (min maxOffset (max minOffset (+ offset (* 0.04 amount)))))
  (format #t "minoffset: ~a, maxoffset: ~a, offset: ~a\n" minOffset maxOffset offset)
  (refreshDepGraph)   
	(onGraphChange)
)


(define (isMatchingDrawElement gameobj)
	(define objname (gameobj-name gameobj))
	(define sceneId (list-sceneid (gameobj-id gameobj)))
	(format #t "objname = ~a, sceneid = ~a\n" objname sceneId)
	(lambda(drawElement)
		(let (
				(drawElementName (car drawElement))
				(drawElementSceneId (cadr drawElement))
			)
			(and (equal? drawElementName objname) (equal? sceneId drawElementSceneId))
		)
	)
)
(define (selectedIndexForGameobj gameobj)
	(define drawElements (car  (getDrawList)))
	(define matchGameObj (isMatchingDrawElement gameobj))
	(define newSelectIndex (list-index matchGameObj drawElements))
	;(define value (list-ref drawElements newSelectIndex))
	;(format #t "draw list: ~a\n" drawElements)
	;(format #t "newSelectIndex: ~a\n" newSelectIndex)
	;(format #t "value: ~a\n" value)
	newSelectIndex
)
(define (onObjSelected gameobj color)
	(onObjectSelected gameobj color)
)

(define (onMapping index)
	(define selectedIndexForMapping (baseNumberToSelectedIndex index))
	(format #t "mapping: ~a, mapping: ~a\n" index selectedIndexForMapping)
	(if selectedIndexForMapping
		(let (
				(isToggle (not (cadr selectedIndexForMapping)))
				(mappedIndex (car selectedIndexForMapping))
			)
			(if isToggle
				(begin
					;(refreshDepGraph)
	  			(setSelectedIndex mappedIndex)
	  			(refreshGraphData)   
	   			(toggleExpanded selectedName)
	  			(onGraphChange)
				)
				(begin
					;(refreshDepGraph)
	  			(setSelectedIndex mappedIndex)
	  			(refreshGraphData)   
	  			(handleItemSelected selectedElement #f)
	  			(onGraphChange)
				)
			)
		)
	)
	;(format #t "selected_index = ~a, selected_name = ~a, selected_element = ~a\n" selectedIndex selectedName selectedElement)
)