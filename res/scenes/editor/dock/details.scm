; 1. details-binding:<attribute value>, this makes the value property get populated with the value in this
; 2. details-group:<attribute value>, details-index: <attributevalue>,  
; ==> if element is selected with same group, the corresponding tint for all other elements is set low if detail-index is different, else high
; 3. if dialog-button-action message is sent, will perform the action in value of attribute button-action for the corresponding object id in the message
; 4. manages value field (as text editor style functionality) for focused elements with details-editabletext:true

(define (assert trueOrFalse message)
  (if (not trueOrFalse) 
    (begin
      (format #t "assertion error: ~a\n" message)
      (exit 1)
    )
  )
)

(define (isSubmitKey key) (equal? key 47))   ; /
(define (isControlKey key) 
  (or 
    (isSubmitKey key) 
    (equal? key 44) 
    (equal? key 257) ; enter key
  ) 
)

(define (submitAndPopulateData)
  (submitData)
  (populateData)
)

(define (queryUpdateForBinding bindingQueryPair) 
  (lambda(updatedValue)
    (let (
        (sqlForUpdate (assoc (car updatedValue) bindingQueryPair))
      )
      (if sqlForUpdate
        (list (cadr updatedValue) (caddr sqlForUpdate))
        #f
      )
    )
  )
)

(define (template sourceString template templateValue)
  (define index (string-contains sourceString template))
  (if index
    (string-replace sourceString templateValue index (+ index (string-length template)))
    sourceString
  )
)




(define (bind-query-main query)  
  (define templatedQuery query)
  (for-each 
    (lambda(dataValue)
      (let ((key (car dataValue)) (value (cadr dataValue)))
        (format #t "data value, key = ~a, value = ~a\n" key value)
        (format #t "template: ~a\n" templatedQuery)
        (set! templatedQuery (template templatedQuery (string-append "$" key) (sqlMakeTypeCorrect value)))
      )
    ) 
    dataValues
  )
  (format #t "bind-query data valus: ~a" dataValues)
  templatedQuery
)

(define (bind-query query value)  ; would be nice to have something like (sql-compile "select $0 from $1" (list value0 value1))
  (template (bind-query-main query) "$VALUE" (if value value "no_data"))
)

(define (serializeVec vec) 
  (format #t "serialize vec: ~a\n" vec)
  (format #t "types: ~a" (string-join (map typeof vec) ", "))
  (string-join (map number->string vec) "?")
)
(define (sqlMakeTypeCorrect templateValue)
  (format #t "template value: ~a, ~a\n" templateValue (typeof templateValue))
  (cond
    ((number? templateValue) 
      (begin
        (format #t "~a is a number\n" templateValue)
        (number->string templateValue)
      )
    )
    ((list? templateValue) (serializeVec templateValue))
    (#t templateValue)
  )
)
(define (submitSqlUpdates updatedValues)
  (define allQueriesObj (lsobj-attr "sql-query"))
  (define bindingQueryPair (map bindingAndQueryFromObj allQueriesObj))
  (define queryUpdateWithValue (filter (lambda(x) x) (map (queryUpdateForBinding bindingQueryPair) updatedValues)))
  (format #t "updated values: ~a\n" updatedValues)
  ;(format #t "queryUpdateWithValue : ~a\n" queryUpdateWithValue)
  (for-each 
    (lambda(updatedValue)
      (let* (
        (templateValue (car updatedValue)) 
        (typeCorrectValue (sqlMakeTypeCorrect templateValue))
        (validQuery 
          (not (or 
            (equal? 0 (string-length typeCorrectValue))
            (> (length (string-split typeCorrectValue #\ )) 1) ; cannot handle spaces yet!
          ))
        )
      )
        (format #t "query: valid = ~a, query ~a\n" validQuery typeCorrectValue)
        (if validQuery
          (sql (sql-compile (bind-query (cadr updatedValue) typeCorrectValue)))
          (format #t "invalid template value\n")
        )
      )
      ;(format #t "updated value: ~a\n" updatedValue)
      ;(format #t "update query: ~a\n" (cadr updatedValue))
    )
    queryUpdateWithValue
  )
)

(define (attrFromPrefixAttr value)
  (define attribute (car value))
  (define payload (cadr value))
  (define index (string-index attribute #\:))
  (if index
    (let* 
      ( (strLength (string-length attribute))
        (prefix (substring attribute 0 index))
        (substr (substring attribute (min strLength (+ index 1)) strLength))
      )
      (if (and index (> (string-length substr) 0))
        (list prefix (list substr payload))
        #f
      )
    )
    #f
  )
)
(define (getPrefixAttr updatedValues prefix)
  (define updatedAttrs (map attrFromPrefixAttr updatedValues))
  (define filterUpdated (filter (lambda(val) (and val (equal? prefix (car val)))) updatedAttrs))
  (define values (map (lambda (val) (cadr val)) filterUpdated))
  values
)

(define (formatWorldAttr worldattr)
  (define prefix (string-split (car worldattr) #\:))
  (define name (car prefix))
  (define attribute (cadr prefix))
  (define value (cadr worldattr))
  (list name attribute value)
)
(define (submitWorldAttr worldattr)
  (define values (map formatWorldAttr worldattr))
  (set-wstate values)

)

(define (maybe-serialize-vec2 val)
  (define key (car val))
  (define value (cadr val))
  (format #t "val is: ~a\n" value)
  (if (and (list? value) (equal? (length value) 2))
    (list key (string-join (map number->string value) " "))
    (list key value)
  )
)
(define (submitData)
  (if managedObj
    (begin
      (let* (
        (updatedValues (map maybe-serialize-vec2 (filterUpdatedObjectValues))) 
        (objattr (getPrefixAttr updatedValues "gameobj"))
        (worldattr (getPrefixAttr updatedValues "world"))
      )
        (if (gameobj-name managedObj)
          (begin
            (gameobj-setattr! managedObj objattr)
            (format #t "set attr: ~a ~a\n" (gameobj-name managedObj) objattr)
            (format #t "world attr: ~a\n" worldattr)
          )
        )
        (submitWorldAttr worldattr)
        (submitSqlUpdates updatedValues)
      )
    )
  )
)

(define (typeof val)
  (cond
    ((number? val) "number")
    ((string? val) "string")
    ((list? val)  (format #f "list(~a)" (length val)))
    (#t "unknown")
  )
)
(define (attrToPretty val) (format #f "~a, type is = ~a\n" val (typeof (cadr val))))
(define (prettyPrint attr)
  (for-each 
    (lambda(val) 
      (format #t (attrToPretty val))
    )
    attr
  )
)
(define (onKeyChar key)
  (if (equal? key 44) ; comma
    (begin
      (format #t "pretty print: \n")
      (prettyPrint dataValues)
      (format #t "\n")
    )
    
  )
  (if (isSubmitKey key) (submitAndPopulateData))
)


(define dataValues (list))

(define (isUpdatedObjectValue dataValue) 
  ;(format #t "is updated object value: ~a\n" dataValue)
  (equal? #t (caddr dataValue))
)
(define (getAttr dataValue) (list (car dataValue) (cadr dataValue)))
(define (filterUpdatedObjectValues) (map getAttr (filter isUpdatedObjectValue dataValues)))

(define (clearStore) (set! dataValues (list)))
(define (updateStoreValueModified keyvalue modified)
  (define key (car keyvalue))
  (define value (cadr keyvalue))  ; maybe guess the type here?
  (define newDataValues (delete (assoc key dataValues) dataValues))
  (set! dataValues (cons (list key value modified) newDataValues))
)
(define (updateStoreValue keyvalue) (updateStoreValueModified keyvalue #f))

(define (executeQuery query) 
  (define queryCast (list-ref query 3))
  (list 
    (car query) 
    (sql (sql-compile (bind-query-main (cadr query))))
    queryCast
  )
)
(define (extractFirstElement bindingName data type)
  (define firstRow (car data))
  (if (> (length firstRow) 0)
    (updateStoreValue (list bindingName (castData (car firstRow) type)))
  )
)

(define (parseVec val)
  (define listParts (string-split val #\?))
  (define numVals (map string->number listParts))
  numVals
)
(define (castData val type)
  (format #t "cast data: val = ~a, type = ~a\n" val type)
  (cond 
    ((equal? type "number") (string->number val))
    ((equal? type "vec")   (parseVec val))
    (#t val)
  )
)
(define (populateSqlResults result) 
  (if result
    (let* ((bindingName (car result)) (data (cadr result)))
      (if data
        (extractFirstElement bindingName data (caddr result))
        (format #t "warning no data for sql binding: ~a\n" bindingName)
      )
    )
  )
)

(define (bindingAndQueryFromObj obj)
  (define attr (gameobj-attr obj))
  (define sqlBinding (assoc "sql-binding" attr))
  (define sqlQuery (assoc "sql-query" attr))
  (define sqlUpdate (assoc "sql-update" attr))
  (define sqlCast (assoc "sql-cast" attr))
  (define binding (if sqlBinding (cadr sqlBinding) #f))
  (define query (if sqlQuery (cadr sqlQuery) #f))
  (define update (if sqlUpdate (cadr sqlUpdate) #f))
  (define cast (if sqlCast (cadr sqlCast) #f))
  (if (and binding query)
    (list binding query update cast)
    #f
  )
)

(define (populateSqlData)
  (define allQueriesObj (lsobj-attr "sql-query"))
  (define bindingQueryPair (map bindingAndQueryFromObj allQueriesObj))
  (define queries (filter (lambda(x) x) bindingQueryPair))
  (define results (map executeQuery queries))
  (for-each populateSqlResults results)
  (format #t "querypair: ~a\n" bindingQueryPair)
)

(define (getRefillStoreWorldValue stateValue)
  (define object (car stateValue))
  (define attribute (cadr stateValue))
  (define value (caddr stateValue))
  (define name (string-append "world:" object ":" attribute))
  (define storeValue (list name value))
  storeValue
)
(define (getRefillGameobjAttr attr)
  (define attribute (car attr))
  (define value (cadr attr))
  (define name (string-append "gameobj:" attribute))
  (format #t "gameobj attr: ~a\n" (attrToPretty attr))
  (list name value)
)

(define (maybe-parse-vec2 val)
  (define listParts (string-split val #\ ))
  (define toNumber (map string->number listParts))
  (define allNumbers (equal? (length toNumber) (length (filter (lambda(x) x) toNumber))))
  (if allNumbers
    toNumber
    val
  )
)
(define (refillCorrectType value)
  (define val (cadr value))
  (define typeCorrectVal 
    (if (string? val)
      (maybe-parse-vec2 val)
      val
    )
  )
  (list (car value) typeCorrectVal)
)
(define (refillStore gameobj)
  (clearStore)
  (updateStoreValue (list "object_name" (gameobj-name gameobj)))
  (format #t "store: all attrs are: ~a\n" (gameobj-attr gameobj))
  (map updateStoreValue (map refillCorrectType (map getRefillGameobjAttr (gameobj-attr gameobj))))

  (updateStoreValue (list "meta-numscenes" (number->string (length (list-scenes)))))
  (updateStoreValue (list "runtime-id" (number->string (gameobj-id gameobj))))
  (updateStoreValue (list "runtime-sceneid" (number->string (list-sceneid (gameobj-id gameobj)))))

  (updateStoreValue (list "play-mode-on" (if playModeEnabled "on" "off")))
  (updateStoreValue (list "pause-mode-on" (if pauseModeEnabled "on" "off")))

  (for-each updateStoreValue (map refillCorrectType (map getRefillStoreWorldValue (get-wstate))))

  (populateSqlData)
)

(define (uniqueName) (number->string (random 1000000)))

(define activeSceneId #f)
(define (createCamera) 
  (if activeSceneId 
    (mk-obj-attr (string-append ">camera-" (uniqueName)) (list) activeSceneId)
  )
)
(define (createLight) 
  (if activeSceneId
    (mk-obj-attr (string-append "!light-" (uniqueName)) (list) activeSceneId)
  )
)
(define (createText) 
  (if activeSceneId 
    (mk-obj-attr (string-append ")text-" (uniqueName)) (list (list "value" "sample text")) activeSceneId)
  )
)

(define (createGeo)
  (if activeSceneId 
    (mk-obj-attr (string-append "<geo-" (uniqueName)) (list) activeSceneId)
  )
)

(define (createPortal)
  (if activeSceneId 
    (mk-obj-attr (string-append "@portal-" (uniqueName)) (list) activeSceneId)
  )
)

(define (createHeightmap)
  (if activeSceneId 
    (mk-obj-attr (string-append "-heightmap-" (uniqueName)) 
      (list
        (list "map" "./res/heightmaps/default.jpg")
      ) 
      activeSceneId
    )
  )
)

(define (createVoxels)
  (if activeSceneId 
    (mk-obj-attr (string-append "]voxel-" (uniqueName)) 
      (list
        (list "from" "2|2|2|11001111")
        (list "fromtextures" "./res/brush/border_5x5.png,./res/heightmaps/dunes.jpg|10201020")
      ) 
      activeSceneId
    )
  )
)

(define (setManipulatorMode mode) (set-wstate (list (list "tools" "manipulator-mode" mode) )))
(define (setAxis axis) (set-wstate (list (list "tools" "manipulator-axis" axis))))

(define pauseModeEnabled #t)

(define (setPauseMode enabled)
  (set! pauseModeEnabled enabled)
  (set-wstate (list
    (list "world" "paused" (if pauseModeEnabled "true" "false"))
  ))
  (sendnotify "alert" (string-append "pause mode " (if pauseModeEnabled "enabled" "disabled"))) 
  (updateStoreValue (list "pause-mode-on" (if pauseModeEnabled "on" "off")))
)
(define (togglePauseMode)
 (setPauseMode (not pauseModeEnabled))
)

(define playModeEnabled #f)
(define (togglePlayMode)
  (if playModeEnabled
    (begin
      (setPauseMode #t)
      (for-each reset-scene (list-scenes (list "editable")))
    )
    (begin
      (setPauseMode #f)
    )
  )
  (set! playModeEnabled (not playModeEnabled))
  (updateStoreValue (list "play-mode-on" (if playModeEnabled "on" "off")))
  (format #t "play mode: ~a" (if playModeEnabled "true" "false"))
  (sendnotify "alert" (format #f "play mode: ~a" (if playModeEnabled "true" "false"))) 
  (sendnotify "play-mode" (if playModeEnabled "true" "false")) 
)

(define (worldStateEqual worldstate key attribute) 
  (define keyValue (car worldstate))
  (define attributeValue (cadr worldstate))
  (and (equal? keyValue key) (equal? attributeValue attribute))
)
(define (getWorldValue worldstates key attribute)
  (define values (filter (lambda(worldstate) (worldStateEqual worldstate key attribute)) worldstates))
  (if (> (length values) 0)
    (car values)
    #f
  )
)
(define (toggleWorldValue key attribute)
  (define worldState (get-wstate))
  (define value (caddr (getWorldValue worldState key attribute)))
  (define isEnabled (not (equal? value "true")))
  (set-wstate (list
    (list key attribute (if isEnabled "true" "false"))
  ))
)
(define (getToggleWorldValue key attribute)
  (lambda() 
    (toggleWorldValue key attribute)
  )
)

(define buttonToAction
  (list
    (list "create-camera" createCamera)
    (list "create-light" createLight)
    (list "create-text" createText)
    (list "create-geo" createGeo)
    (list "create-portal" createPortal)
    (list "create-heightmap" createHeightmap)
    (list "create-voxels" createVoxels)
    (list "set-transform-mode" (lambda() (setManipulatorMode "translate")))
    (list "set-scale-mode" (lambda() (setManipulatorMode "scale")))
    (list "set-rotate-mode" (lambda() (setManipulatorMode "rotate")))
    (list "toggle-play-mode" togglePlayMode)
    (list "toggle-pause-mode" togglePauseMode)
    (list "set-axis-x" (lambda() (setAxis "x")))
    (list "set-axis-y" (lambda() (setAxis "y")))
    (list "set-axis-z" (lambda() (setAxis "z")))
    (list "copy-object" (lambda() (sendnotify "copy-object" "true")))
  )
)

(define mainSceneId (list-sceneid (gameobj-id mainobj)))
(define mainpanelId (gameobj-id (lsobj-name "(test_panel")))

(define (isManagedText gameobj)
  (and 
    (shouldUpdateText (gameobj-attr gameobj))
    (equal? ")" (substring (gameobj-name gameobj) 0 1))
  )
)

(define (getDataValue attrField) 
  (define value (assoc attrField dataValues))
  (if value (cadr value) #f)
)

(define (makeTypeCorrect oldvalue newvalue)
  (format #t "make type correct: ~a ~a\n" oldvalue newvalue)
  (if (number? oldvalue)
    (if (string? newvalue) 
      (if 
        (or 
          (equal? (string-length newvalue) 0) 
          (equal? "-" (substring newvalue 0 (string-length newvalue)))
          (equal? "." (substring newvalue 0 (string-length newvalue)))
        ) 
        0 (string->number newvalue)
      ) 
      newvalue
    )
    newvalue
  )
)

(define (splitVec value)
  (define values (string-split value #\ ))
  (define isNumber (map string->number values))
  (define numbers (filter (lambda(x) (not (equal? x #f))) isNumber))
  (if (equal? (length numbers) 3) value #f)
)
(define (fixList value)
  (define values (string-split value #\|))
  (define splitVecs (map splitVec values))
  (define filteredVecs (filter (lambda(x) (not (equal? x #f))) splitVecs))
  (define joinedValues (string-join filteredVecs "|"))
  (format #t "fix list: values: ~a ~a\n" (length values) values)
  ;(format #t "fix list: filtered vecs: ~a\n" filteredVecs)
  
;  joinedValues
  (format #t "fix list: splitVecs: ~a\n" splitVecs)
  (format #t "fix list: filtered vecs: ~a\n\n" filteredVecs)
  (format #t "fix list: joined values: ~a\n" joinedValues)
  joinedValues
)
(define (getUpdatedValue detailBindingName detailBindingIndex detailBindingType newvalueOld)
  (define oldvalue (getDataValue detailBindingName))
  (define newvalue (if (equal? detailBindingType "list") (fixList newvalueOld) newvalueOld))
  (if detailBindingIndex
      (begin
        (if (equal? oldvalue #f)
          (set! oldvalue (list 0 0 0 0))  ; should come from some type hint
        )
        (list-set! oldvalue detailBindingIndex (makeTypeCorrect (list-ref oldvalue detailBindingIndex) newvalue))
        (format #t "old value: ~a ~a\n"  oldvalue (map number? oldvalue))
        (list detailBindingName oldvalue)
      )
      (list detailBindingName (makeTypeCorrect oldvalue newvalue))
  )    
  
)

(define (newCursorIndex eventType oldIndex newTextLength oldoffset wrapAmount oldCursorDir oldHighlightLength) ;text wrapAmount key offset) 
  (define distanceOnLine (- oldIndex oldoffset))
  (define lastEndingOnRight (= distanceOnLine (- wrapAmount 1)))
  (define lastEndingOnLeft (= distanceOnLine 0))
  (define oldCursorDirLeft (equal? oldCursorDir "left"))
  (define newCursorDir oldCursorDir)
  (define highlightLength oldHighlightLength)
  (define index 
    (cond 
      ((equal? eventType 'up) 
        (begin
          (set! highlightLength (min (- newTextLength (if oldCursorDirLeft oldIndex (+ oldIndex 1))) (+ highlightLength 1))) 
          (format #t "event up: highlight: ~a\n" highlightLength)
          oldIndex
        )  
      )
      ((equal? eventType 'down) (set! highlightLength (max 0 (- highlightLength 1))) oldIndex)
      ((equal? eventType 'selectAll) (begin
        (set! highlightLength newTextLength)
        (set! newCursorDir "left")
        0
      ))
      ((or (equal? eventType 'left)   (and (equal? eventType 'backspace) (<= oldHighlightLength 0))) 
        (if (not oldCursorDirLeft)
          (begin
            (set! newCursorDir "left")
            oldIndex
          )
          (max 0 (- oldIndex 1))
        )
      )
      ((or (equal? eventType 'insert) (equal? eventType 'right)) 
        (if (and lastEndingOnRight oldCursorDirLeft)  ; make sure this doesn't exceed the length
          (begin
            (set! newCursorDir "right")
            (min newTextLength oldIndex)
          )
          (min (if (equal? newCursorDir "right") (- newTextLength 1) newTextLength) (+ oldIndex 1))
        )
      )
      (#t oldIndex)
    )
  )
  (if (> highlightLength 0)
    (cond
      ((equal? eventType 'left)
        (set! index oldIndex)
      )
      ((equal? eventType 'right)
        (set! index (+ oldIndex oldHighlightLength))
      )
    )
  )

  (if (or (equal? eventType 'right) (equal? eventType 'left) (equal? eventType 'insert) (equal? eventType 'backspace) (equal? eventType 'delete))
    (set! highlightLength 0)
  )

  (format #t "last ending: (~a, ~a)\n" lastEndingOnLeft lastEndingOnRight)
  (format #t "distance on line: ~a\n" distanceOnLine)
  (list index newCursorDir highlightLength)
)


; todo -> take into account when the text is deleted 
(define (newOffsetIndex type oldoffset cursor wrapAmount strlen)
  (define rawCursorIndex (car cursor))
  (define newCursorIndex (if (equal? (cadr cursor) "left") rawCursorIndex (+ rawCursorIndex 1)))
  (define cursorHighlight (caddr cursor))
  (define cursorFromOffset (- (+ newCursorIndex cursorHighlight) oldoffset))
  (define wrapRemaining (- wrapAmount cursorFromOffset))
  (define cursorOverLeftSide (> wrapRemaining wrapAmount))
  (define cursorOverRightSide (< wrapRemaining 0))
  (define amountOverLeftSide (- wrapRemaining wrapAmount))
  (define amountOverRightSide (* -1 wrapRemaining))
  (define newOffset   
    (cond 
      (cursorOverRightSide (+ oldoffset amountOverRightSide))
      (cursorOverLeftSide  (- oldoffset amountOverLeftSide))
      (#t oldoffset)
    )
  )
  (define numCharsLeft (- strlen newOffset))
  (define diffFromWrap (- numCharsLeft wrapAmount))
  (define finalOffset (if (<= diffFromWrap 0)
    (+ newOffset diffFromWrap)
    newOffset
  ))

  (format #t "offset index, highlight: ~a\n" cursorHighlight)
  finalOffset
)

(define (updateText obj text cursor offset)
  (define cursorIndex (car cursor))
  (define cursorDir (cadr cursor))
  (define cursorHighlightLength (caddr cursor))
  (define objattr (gameobj-attr obj))
  (define detailBindingPair (assoc "details-binding" objattr))
  (define detailBindingIndexPair (assoc "details-binding-index" objattr))
  (define detailBinding (if detailBindingPair (cadr detailBindingPair) #f))
  (define detailBindingIndex (if detailBindingIndexPair (inexact->exact (cadr detailBindingIndexPair)) #f))
  (define editableTypePair (assoc "details-editable-type" objattr))
  (define editableType (if editableTypePair (cadr editableTypePair) #f))

  (define newValues 
    (list
      (list "offset" offset)
      (list "cursor" cursorIndex)
      (list "cursor-dir" cursorDir)
      (list "cursor-highlight" cursorHighlightLength)
      (list "value" text)
    )
  )

  (format #t "cursor highlight: ~a\n" cursorHighlightLength)
  (format #t "updating text: ~a\n" text)

  (gameobj-setattr! obj newValues)
  (if detailBinding
    (updateStoreValueModified (getUpdatedValue detailBinding detailBindingIndex editableType text) #t)
  )
  (format #t "cursor is: ~a\n" cursor)
)

(define (appendString currentText key cursorIndex highlightLength)
  (define _ (format #t "currentText: ~a, key: ~a, cursorIndex: ~a, highlightLength = ~a\n" currentText key cursorIndex highlightLength))
  (define length (string-length currentText))
  (define splitIndex (min length cursorIndex))
  (define start (substring currentText 0 splitIndex))
  (define end   (substring currentText (+ splitIndex highlightLength) length))
  (define __ (format #t "length: ~a, splitIndex = ~a, hihglightLength = ~a\n" length splitIndex highlightLength))
  (string-append start (string (integer->char key)) end)
)
(define (deleteChar currentText cursorIndex highlightLength)
  (define length (string-length currentText))
  (if (and (> cursorIndex 0) (< cursorIndex (+ 1 length)))
    (let* (
      (splitIndex (min length (- cursorIndex 1)))
      (start (substring currentText 0 splitIndex))
      (end (substring currentText (if (equal? highlightLength 0) (+ 1 splitIndex) (+ splitIndex highlightLength)) length))
    )
      (string-append start end)
    )
    currentText
  )
)

(define (getUpdatedText attr obj key cursorIndex cursorDir highlightLength eventType)
  (define effectiveIndex (if (equal? cursorDir "left") cursorIndex (+ cursorIndex 1)))
  (define currentText (cadr (assoc "value" attr)))
  (format #t "highlight length: ~a\n" highlightLength)
  (cond 
    ((equal? eventType 'backspace) (set! currentText (deleteChar currentText (if (<= highlightLength 0) effectiveIndex (+ effectiveIndex 1)) highlightLength)))
    ((equal? eventType 'delete) (set! currentText (deleteChar currentText (+ effectiveIndex 1) highlightLength)))
    ((or (equal? eventType 'selectAll) (equal? eventType 'left) (equal? eventType 'right) (equal? eventType 'up) (equal? eventType 'down)) #t)
    (#t 
      (begin
       ;(format #t "key is ~a ~a\n" key (string (integer->char key)))
        (set! currentText (appendString currentText key effectiveIndex highlightLength))
      )
    )
  )
  currentText
)


(define (shouldUpdateText attr)
  (define editableText (assoc "details-editabletext" attr))
  (and editableText (equal? "true" (cadr editableText)))
)
(define (isEditableType type attr) 
  (define editableText (assoc "details-editable-type" attr))
  (define isEditableType (if editableText (equal? (cadr editableText) type) #f))
  (format #t "is editable type: ~a ~a\n" type isEditableType)
  isEditableType
)

(define (getUpdateType key)
  (cond
    ((equal? key 259) 'backspace)
    ((equal? key 96)  'delete) ; ~ sign, should change to delete but will delete the item
    ((equal? key 263) 'left)
    ((equal? key 262) 'right)
    ((equal? key 265) 'up)
    ((equal? key 264) 'down)
    ((equal? key 344) 'selectAll) ;shift
    (#t 'insert)
  )
)

(define (isZeroLength text) (equal? 0 (string-length text)))
(define (isNumber text) (if (string->number text) #t #f))
(define (isNegativePrefix text)
  (define isNegative 
    (and 
      (equal? 1 (string-length text))
      (equal? "-" (substring text 0 1))
    )
  )
  (format #t "is negative: ~a ~a\n" text isNegative)
  isNegative
)
(define (isDecimal text) (and (equal? 1 (string-length text)) (equal? (substring text 0 1) ".")))
(define (isPositiveNumber text) 
  (define number (string->number text))
  (if number
    (>= number 0)
    #f
  )
)
(define (isInteger text) (exact-integer? (string->number text)))
(define (isPositiveInteger text) 
  (define num (string->number text))
  (and
    (if num (>= num 0) #f )
    (exact-integer? num)
  )
)

(define (isTypeNumber text) (or (isZeroLength text) (isNumber text) (isDecimal text) (isNegativePrefix text)))
(define (isTypePositiveNumber text) (or (isZeroLength text) (isDecimal text) (isPositiveNumber text)))
(define (isTypeInteger text) (or (isZeroLength text) (isInteger text) (isNegativePrefix text)))
(define (isTypePositiveInteger text) (or (isZeroLength text) (isPositiveInteger text)))
(define (shouldUpdateType newText attr)
  (define update (cond
    ((isEditableType "number" attr) 
      (format #t "evaluating number type!\n")
      (isTypeNumber newText)
    )
    ((isEditableType "positive-number" attr) 
      (format #t "evaluating positive number type!\n")
      (isTypePositiveNumber newText)
    )
    ((isEditableType "integer" attr)
      (isTypeInteger newText)
    )
    ((isEditableType "positive-integer" attr)
      (isTypePositiveInteger newText)
    )
    (#t #t)
  ))
  (format #t "should update: (~a) (~a)\n" newText update)
  update
)

(define focusedElement #f)
(define (processFocusedElement key)
  ;(format #t "focused element: top\n")
  (if focusedElement
    (begin
      (let ((attr (gameobj-attr focusedElement)))
        (if (shouldUpdateText attr)
          (begin
            ;(format #t "focused element: should update\n")
            (let* (
              (cursorIndex (inexact->exact (cadr (assoc "cursor" attr))))
              (oldCursorDir (cadr (assoc "cursor-dir" attr)))
              (oldHighlight (inexact->exact (cadr (assoc "cursor-highlight" attr))))
              (offsetIndex (inexact->exact (cadr (assoc "offset" attr)))) 
              (updateType (getUpdateType key))
              (wrapAmount (inexact->exact (cadr (assoc "wrapamount" attr))))
              (newText (getUpdatedText (gameobj-attr focusedElement) focusedElement key cursorIndex oldCursorDir oldHighlight updateType))
              (cursor (newCursorIndex updateType cursorIndex (string-length newText) offsetIndex wrapAmount oldCursorDir oldHighlight))
              (offset (newOffsetIndex updateType offsetIndex cursor wrapAmount (string-length newText)))
            )              
              (if (shouldUpdateType newText attr)
                (updateText focusedElement newText cursor offset)
              )
            )
            
          )
          ;(format #t "focused element: should not update\n")
        )
      )
    )
  )
  (submitData)
)

(define (calcSlideValue objattr percentage)
  (define min (cadr (assoc "min" objattr)))
  (define max (cadr (assoc "max" objattr)))
  (define range (- max min))
  (+ min (* range percentage))
)
(define (uncalcSlideValue obj value)
  (define objattr (gameobj-attr obj))
  (define min (cadr (assoc "min" objattr)))
  (define max (cadr (assoc "max" objattr)))
  (define ratio (/ (- value min) (- max min)))
  ;(format #t "min = ~a, max = ~a, range = ~a, ratio = ~a\n" min max range ratio)
  ratio
)

(define (updateBindingWithValue value objattr)
  (define detailBindingPair (assoc "details-binding" objattr))
  (define detailBindingIndexPair (assoc "details-binding-index" objattr))
  (define detailBinding (if detailBindingPair (cadr detailBindingPair) #f))
  (define detailBindingIndex (if detailBindingIndexPair (inexact->exact (cadr detailBindingIndexPair)) #f))
  (define editableTypePair (assoc "details-editable-type" objattr))
  (define editableType (if editableTypePair (cadr editableTypePair) #f))

  ;(format #t "values: ~a ~a ~a\n" (car objvalues) (cadr objvalues) (caddr objvalues))
  (if detailBinding 
    (updateStoreValueModified (getUpdatedValue detailBinding detailBindingIndex editableType value) #t)
  )
)

(define (onSlide objvalues)
  (define obj (car objvalues))
  (define slideAmount (cadr objvalues))
  (define objattr (caddr objvalues))
  (define value (calcSlideValue objattr slideAmount))
  (updateBindingWithValue value objattr)
  (submitAndPopulateData)
)

(define (isSelectableItem layerAttr) (if layerAttr (not (equal? "basicui" (cadr layerAttr))) #t))

(define (maybe-perform-action objattr)
  (define attrActions (assoc "details-action" objattr))
  ;(format #t "attr actions: ~a\n " attrActions)
  (if (and attrActions (controlEnabled objattr))
    (let ((action (assoc (cadr attrActions) buttonToAction)))
      (if action
        ((cadr action))
        (format #t "no action for ~a\n" (cadr attrActions))
      )
    )
  )
)

(define (maybe-set-binding objattr)
  (define shouldSet (if (assoc "details-binding-set" objattr) #t #f))
  (define detailBindingPair (assoc "details-binding-toggle" objattr))
  (define detailBindingIndexPair (assoc "details-binding-index" objattr))
  (define detailBinding (if detailBindingPair (cadr detailBindingPair) #f))
  (define detailBindingIndex (if detailBindingIndexPair (inexact->exact (cadr detailBindingIndexPair)) #f))
  (define bindingOn (assoc "details-binding-on" objattr))
  (define enableValue (if bindingOn (cadr bindingOn) #f))
  (define editableTypePair (assoc "details-editable-type" objattr))
  (define editableType (if editableTypePair (cadr editableTypePair) #f))

  ;;(format #t "shouldset = ~a, enableValue = ~a, detailBinding = ~a\n" shouldSet enableValue detailBinding)
  (if (and shouldSet enableValue detailBinding) 
    (updateStoreValueModified (getUpdatedValue detailBinding detailBindingIndex editableType enableValue) #t)
  )
  (submitAndPopulateData)
)

(define (maybe-set-text-cursor gameobj)
  (define objattr (gameobj-attr gameobj))
  (define value (assoc "value" objattr))
  (define wrap (assoc "wrapamount" objattr))
  (define offset (assoc "offset" objattr))

  (define wrapAmount (if wrap (cadr wrap) #f))
  (define text (if value (cadr value) #f))
  (define offsetValue (if offset (cadr offset) #f))
  (if (and text wrapAmount offsetValue)
    (let ((cursorValue (min (- (max 1 (string-length text)) 1) (+ (- wrapAmount 1) offsetValue))))
      (assert (>= cursorValue 0) (format #f "name: ~a => cursor value is expected to be >= 0 (got = ~a), wrapAmount = ~a, text = ~a, offset = ~a\n" (gameobj-name gameobj) cursorValue wrapAmount text offsetValue))
      (gameobj-setattr! gameobj 
        (list
          (list "cursor" cursorValue)
          (list "cursor-dir" "right")
          (list "cursor-highlight" 0)
        )
      )
    )
  )
)

(define (unsetFocused)
  (if focusedElement
    (gameobj-setattr! focusedElement 
      (list 
        (list "value" "")
        (list "tint" (list 1 1 1 1))
        (list "cursor" -1)             ; probably shouldn't be setting this for every element
        (list "cursor-dir" "left")
        (list "cursor-highlight" 0)
      )
    )
  )
  (set! focusedElement #f)    
)

(define managedTextSelectionMode #f)
(define (setManagedSelectionMode textobj)
  (format #t "set managed selection mode: ~a\n" textobj)
  (set! managedTextSelectionMode textobj)
)

(define (finishManagedSelectionMode obj)
  (format #t "finishManagedSelectionMode end: ~a ~a\n" (gameobj-name obj) managedTextSelectionMode)
  (updateText managedTextSelectionMode (gameobj-name obj) (list 0 "left" 0) 0)
  (set! managedTextSelectionMode #f)
)

(define (handleActiveScene sceneId objattr)
  (define layerAttr (assoc "layer" objattr))
  (define layer (if layerAttr (cadr layerAttr) #f))
  (format #t "layer: ~a\n" layer)
  (if (not (equal? layer "basicui"))
    (begin
      (set! activeSceneId sceneId)
      (sendnotify "active-scene-id" (number->string sceneId))
    )
  )
)

(define managedObj #f)
(define (onObjSelected gameobj _)
  (define objattr (gameobj-attr gameobj))
  (define reselectAttr (assoc "details-reselect" objattr))
  (define sceneId (list-sceneid (gameobj-id gameobj)))
  (define objInScene (equal? sceneId (list-sceneid (gameobj-id mainobj))))
  (define managedText (and objInScene (isManagedText gameobj)))
  (define valueIsSelectionType (assoc "details-value-selection" objattr))
  (define valueDialogType (assoc "details-value-dialog" objattr))

  (if (and objInScene reselectAttr)
    (onObjSelected (lsobj-name (cadr reselectAttr)) #f)
    (begin
      (if (equal? (gameobj-id gameobj) (gameobj-id mainobj)) ; assumes script it attached to window x
        (sendnotify "dock-self-remove" (number->string (gameobj-id mainobj)))
      )

      (if valueDialogType
        (sendnotify "explorer" (cadr valueDialogType))
      )
      (if valueIsSelectionType
        (if managedText 
          (setManagedSelectionMode gameobj)
          (format #t "selection type but not text\n")
        )
      )

      (if (not managedTextSelectionMode)
        (begin
          (if managedText
            (begin
              ;(format #t "is is a managed element: ~a\n" (gameobj-name gameobj))
              (unsetFocused)
              (set! focusedElement gameobj)
              (gameobj-setattr! gameobj 
                (list (list "tint" (list 0.3 0.3 0.6 1)))
              )
            )
            (unsetFocused)
          )
          
          (if (isSelectableItem (assoc "layer" objattr))
            (begin
              (refillStore gameobj)
              (set! managedObj gameobj)
              (populateData)
            )
          )
          (maybe-set-binding objattr)
          (if managedText (maybe-set-text-cursor gameobj))
        )
      )
    )
  )
)

(define (onObjUnselected)
  (unsetFocused)
)


(define hoveredObj #f)
(define (onObjHover obj)
  (set! hoveredObj obj)
)
(define (onObjUnhover index)
  (set! hoveredObj #f)
)
(define (onMouse button action mods)
  (if (and (equal? button 0) (equal? action 0) (and hoveredObj))
    (begin
      (handleActiveScene (list-sceneid (gameobj-id hoveredObj)) (gameobj-attr hoveredObj))  ; pull this into a seperate script, don't like the idea of the editor managing this 
      (if (equal? (list-sceneid (gameobj-id hoveredObj)) (list-sceneid (gameobj-id mainobj)))
        (begin
          (maybe-perform-action (gameobj-attr  hoveredObj))
        )
      )
    )
  )
  (if (and (equal? button 1) (equal? action 0) hoveredObj)
    (begin
      (if managedTextSelectionMode
        (begin
          (finishManagedSelectionMode hoveredObj)
          (submitData)
        )
      )
    )
  )
  (populateData)
)

(define (enforceLayouts)
  ;; todo remove - no items in this layout, should require this 
  (enforce-layout (gameobj-id (lsobj-name "(test_panel")))
)
(enforceLayouts)

(define (onKey key scancode action mods)
  ;(format #t "action is: ~a\n" action)
  (if (equal? action 0)
    (begin
      (if (equal? key 59)  ; key = ; 
        (togglePauseMode)
      )
      (if (equal? key 39)  ; key = '
        (togglePlayMode)
      )
    )
  )
  (if (and (or (equal? action 1) (equal? action 2)) (not (isControlKey key)))
    (processFocusedElement key)
  )
)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (create-attr-pair gameobj) (list gameobj (gameobj-attr gameobj)))
(define (get-binded-elements bindingType) (map create-attr-pair (lsobj-attr bindingType)))

(define (not-found-from-attr attrs)
  (define type (assoc "details-editable-type" attrs))
  (set! type (if type (cadr type) type))
  (if (or (equal? type "number") (equal? type "positive-number"))
    (list 0 0 0 0)
    #f
  )
)

(define (extract-binding-element attr-pair bindingType) 
  (define attrs (cadr attr-pair))
  (define detailBindingIndex (assoc "details-binding-index" attrs))
  (list 
    (car attr-pair) 
    (cadr (assoc bindingType attrs))
    (if detailBindingIndex (inexact->exact (cadr detailBindingIndex)) #f)
    (not-found-from-attr attrs)
  )
)
(define (all-obj-to-bindings bindingType) (map (lambda(attrPair) (extract-binding-element attrPair bindingType)) (get-binded-elements bindingType)))

(define (generateGetDataForAttr defaultValue)
  (lambda(attrField) 
    (let ((fieldPair (getDataValue attrField)))
      (if fieldPair fieldPair defaultValue) 
    )
  )
)

(define (getType x)
  (cond 
    ((number? x) "number")
    ((pair? x) "pair")
    ((string? x) "string")
    ((list? x) "list")
  )
)

(define (getGameobjType gameobj)
  (define objname (gameobj-name gameobj))
  (define prefix (substring objname 0 1))
  (cond
    ((equal? prefix "_") "slider")
    ((equal? prefix ")") "text")
  )
)

(define (update-binding attrpair getDataForAttr) 
  (define dataValue (getDataForAttr (cadr attrpair)))
  (define bindingIndex (caddr attrpair))
  (define obj (car attrpair))
  (define objType (getGameobjType obj))
  ;(format #t "binding index: ~a ~a\n" bindingIndex (number? bindingIndex))
  ;(format #t "type for update dataValue is: ~a, value: ~a\n" (getType dataValue) dataValue)

  (if (and bindingIndex (list? dataValue) (< bindingIndex (length dataValue)))
    (set! dataValue (list-ref dataValue bindingIndex))
  )
  (if (number? dataValue)
    (set! dataValue (number->string dataValue))
  )
  ;(format #t "data value: ~a\n" dataValue)
  ;(format #t "update binding for: ~a\n" (gameobj-name obj))
  ;(format #t "datavalues: ~a\n" dataValues)
  ;(format #t "data = ~a, index = ~a\n" dataValue bindingIndex)

  (if dataValue
    (begin
        (cond
          ((equal? objType "text") 
            (gameobj-setattr! obj
              (list (list "value" (if (number? dataValue) (number->string dataValue) dataValue)))
            )
          )
          ((equal? objType "slider")
            (format #t "its a slider!\n")
            ;(assert (number? dataValue) (format #f "slider must use a number as data value, got: ~a ~a\n" (getType dataValue) dataValue))
            (gameobj-setattr! obj
              (list (list "slideamount" (uncalcSlideValue obj (if (string? dataValue) (string->number dataValue) dataValue))))
            )
          )
        )
    )
  )
  
  attrpair
)


(define (controlEnabled gameobjAttr) 
  (define isEnabledBindingPair (assoc "details-enable-binding" gameobjAttr))
  (define isEnabledBinding (if isEnabledBindingPair (cadr isEnabledBindingPair) #f))
  (define isEnabledBindingOffPair (assoc "details-enable-binding-off" gameobjAttr))
  (define isEnabledBindingOff (if isEnabledBindingOffPair (cadr isEnabledBindingOffPair) #f))
  (define bindingValue (getDataValue isEnabledBinding))
  (if (and isEnabledBinding isEnabledBindingOff)
    (not (equal? bindingValue isEnabledBindingOff))
    #t
  )
)

(define (update-toggle-binding attrpair getDataForAttr)
  (define toggleEnableText (getDataForAttr (cadr attrpair)))
  (define gameobj (car attrpair))
  (define gameobjAttr (gameobj-attr gameobj))
  (define bindingOn (assoc "details-binding-on" gameobjAttr))
  (define enableValueStr (if bindingOn (cadr bindingOn) "enabled"))
  (define enableValue (equal? enableValueStr toggleEnableText))
  (define isEnabled (controlEnabled gameobjAttr))

  (define onOffColor (if enableValue (list 0.3 0.3 0.6 1) (list 1 1 1 1)))
  ;(format #t "enable value str is: ~a for name ~a\n" enableValueStr (gameobj-name gameobj))
  ;(format #t "toggle enable text: ~a\n" toggleEnableText)
  ;(format #t "update toggle binding: ~a with value ~a (~a)\n" attrpair enableValue toggleEnableText)
  (gameobj-setattr! gameobj
    (list 
      (list "state" (if enableValue "on" "off"))
      (list "tint"  (if isEnabled onOffColor (list 0.4 0.4 0.4 1)))
    )
  )
)
(define (toggleButtonBinding objid on)
  (define objattr (gameobj-attr (gameobj-by-id objid)))
  (define detailsBinding (assoc "details-binding-toggle" objattr))
  (define onValue (assoc "details-binding-on" objattr))
  (define offValue (assoc "details-binding-off" objattr))
  ;(format #t "on value, off value: ~a\n ~a\n" onValue offValue)
  (if (and detailsBinding on onValue)
    (updateStoreValueModified (list (cadr detailsBinding) (cadr onValue)) #t)
  )
  (if (and detailsBinding (not on) offValue)
    (updateStoreValueModified (list (cadr detailsBinding) (cadr offValue)) #t)
  )
)

(define (notFoundData attrpair)
  (define defaultValue (list-ref attrpair 3))
  (format #t "not found: ~a\n" defaultValue)
  defaultValue
)
(define (populateData)
  (for-each 
    (lambda(attrpair) 
      (update-binding attrpair (generateGetDataForAttr  (notFoundData attrpair)))
    ) 
    (all-obj-to-bindings "details-binding")
  )
  (for-each 
    (lambda(attrpair) 
      ;(update-binding attrpair getDataForAttr)
      (update-toggle-binding attrpair (generateGetDataForAttr  #f))
    ) 
    (all-obj-to-bindings "details-binding-toggle")  ;
  )
  (enforceLayouts)
)


(define (objAttrEqual obj attrKey attrValue)
  (define objattr (gameobj-attr obj))
  (define keyPair (assoc attrKey objattr))
  (format #t "obj attr: ~a\nkeypair: ~a\n\n" objattr keyPair)
  (and keyPair (equal? attrValue (cadr keyPair)))
)
(define (updateDialogValues dialogType value)
  (define allQueriesObj (lsobj-attr "details-value-dialog"))
  (define matchingDialogObjs (filter (lambda(obj) (objAttrEqual obj "details-value-dialog" dialogType)) allQueriesObj))
  (for-each 
    (lambda(obj)  
      (updateBindingWithValue value (gameobj-attr obj))
    ) 
    matchingDialogObjs
  )
)
;;;;;;;;;;;;;;;

(define (getSlidePercentage id)
  (define gameobj (gameobj-by-id id))
  (define gameobjAttr (gameobj-attr gameobj))
  (define slideAmount (assoc "slideamount" gameobjAttr))
  (if slideAmount (list gameobj (cadr slideAmount) gameobjAttr) #f)
)

(define (onMessage key value)
  (if (equal? key "explorer-sound-final")
    (begin
      (updateDialogValues "load-sound" value)
      (submitAndPopulateData)
    )
  )
  (if (equal? key "active-scene-id")
    (set! activeSceneId (string->number value))
  )
  (if (equal? key "editor-button-on")
    (begin
      (toggleButtonBinding (string->number value) #t)
      (submitAndPopulateData) ; remove
    )

  )
  (if (equal? key "editor-button-off")
    (begin
      (toggleButtonBinding (string->number value) #f)
      (submitAndPopulateData) ; remove
    )
  )
  (if (equal? key "details-editable-slide")
    (format #t "slide: ~a\n" (onSlide (getSlidePercentage (string->number value))))
  )
)

