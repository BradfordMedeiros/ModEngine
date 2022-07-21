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
  (format #t "key is: ~a\n" key)
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
(define (submitData)
  (if managedObj
    (begin
      (let ((updatedValues (filterUpdatedObjectValues)))
        (format #t "submiit attrs: ~a\n" updatedValues)
        (gameobj-setattr! managedObj updatedValues)
        ;; temporary, just to get the effect, should change
        (for-each (lambda(attrPair) 
          (if (equal? "editor-eoe-mode" (car attrPair))
            (begin
              (format #t "new eoe mode: ~a\n" (cadr attrPair))
              (format #t "not eoe -> ~a\n" attrPair)
              (if (equal? (cadr attrPair) "enabled") (set! eoeMode #t))
              (if (equal? (cadr attrPair) "disabled") (set! eoeMode #f))
            )
          )
        ) updatedValues)
      )
    )
  )
)
(define (onKeyChar key)
  (format #t "key is: ~a\n" key)
  (if (equal? key 44) ; comma
    (format #t "~a\n" dataValues) 
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

(define (refillStore gameobj)
  (clearStore)
  (updateStoreValue (list "object_name" (gameobj-name gameobj)))
  (format #t "store: all attrs are: ~a\n" (gameobj-attr gameobj))
  (map updateStoreValue (gameobj-attr gameobj))

  (updateStoreValue (list "editor-eoe-mode" (if eoeMode "enabled" "disabled")))
  (updateStoreValue (list "meta-numscenes" (number->string (length (list-scenes)))))
  (updateStoreValue (list "runtime-id" (number->string (gameobj-id gameobj))))
  (updateStoreValue (list "runtime-sceneid" (number->string (list-sceneid (gameobj-id gameobj)))))
)

(define (createCameraPlaceholder) (format #t "placeholder to create camera!\n"))
(define (createLightPlaceholder) (format #t "placeholder to create light!\n"))
(define (setManipulatorMode mode) (set-wstate (list (list "tools" "manipulator-mode" mode) )))

(define buttonToAction
  (list
    (list "create-camera" createCameraPlaceholder)
    (list "create-light" createLightPlaceholder)
    (list "set-transform-mode" (lambda() (setManipulatorMode "translate")))
    (list "set-scale-mode" (lambda() (setManipulatorMode "scale")))
    (list "set-rotate-mode" (lambda() (setManipulatorMode "rotate")))
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
(define (getUpdatedValue detailBindingName detailBindingIndex newvalue)
  (define oldvalue (getDataValue detailBindingName))
  (if detailBindingIndex
    (begin
      (if (equal? oldvalue #f)
        (set! oldvalue (list 0 0 0))  ; should come from some type hint
      )
      (list-set! oldvalue detailBindingIndex (makeTypeCorrect (list-ref oldvalue detailBindingIndex) newvalue))
      (format #t "old value: ~a ~a\n"  oldvalue (map number? oldvalue))
      (list detailBindingName oldvalue)
    )
    (list detailBindingName newvalue)
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
    (updateStoreValueModified (getUpdatedValue detailBinding detailBindingIndex text) #t)
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
        (format #t "key is ~a ~a\n" key (string (integer->char key)))
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

(define (onSlide objvalues)
  (define obj (car objvalues))
  (define slideAmount (cadr objvalues))
  (define objattr (caddr objvalues))
  (define detailBindingPair (assoc "details-binding" objattr))
  (define detailBindingIndexPair (assoc "details-binding-index" objattr))
  (define detailBinding (if detailBindingPair (cadr detailBindingPair) #f))
  (define detailBindingIndex (if detailBindingIndexPair (inexact->exact (cadr detailBindingIndexPair)) #f))
  ;(format #t "values: ~a ~a ~a\n" (car objvalues) (cadr objvalues) (caddr objvalues))
  (if detailBinding 
    (updateStoreValueModified (getUpdatedValue detailBinding detailBindingIndex slideAmount) #t)
  )
  (submitAndPopulateData)
)

(define eoeMode #f)
(define (isSelectableItem layerAttr)
  (if eoeMode 
    (begin
      (updateStoreValue (list "editor-eoe-mode" "disabled"))
      (set! eoeMode #f)
      #t
    )
    (and (not eoeMode)  (if layerAttr (not (equal? "basicui" (cadr layerAttr))) #t))
  )
)

(define (maybe-perform-action objattr)
  (define attrActions (assoc "details-action" objattr))
  ;(format #t "attr actions: ~a\n " attrActions)
  (if attrActions
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
  ;(format #t "shouldset = ~a, enableValue = ~a, detailBinding = ~a\n" shouldSet enableValue detailBinding)
  (if (and shouldSet enableValue detailBinding) 
    (updateStoreValueModified (getUpdatedValue detailBinding detailBindingIndex enableValue) #t)
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

(define managedObj #f)
(define (onObjSelected gameobj _)
  (define objattr (gameobj-attr gameobj))
  (define reselectAttr (assoc "details-reselect" objattr))
  (define objInScene (equal? (list-sceneid (gameobj-id gameobj)) (list-sceneid (gameobj-id mainobj))))
  (define managedText (and objInScene (isManagedText gameobj)))
  (if (and objInScene reselectAttr)
    (onObjSelected (lsobj-name (cadr reselectAttr)) #f)
    (begin
      (if (equal? (gameobj-id gameobj) (gameobj-id mainobj)) ; assumes script it attached to window x
        (sendnotify "dock-self-remove" (number->string (gameobj-id mainobj)))
      )
      (if managedText
        (begin
          ;(format #t "is is a managed element: ~a\n" (gameobj-name gameobj))
          (unsetFocused)
          (set! focusedElement gameobj)
          (gameobj-setattr! gameobj 
            (list (list "tint" (list 0 0 1 1)))
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
      (maybe-perform-action objattr)
      (maybe-set-binding objattr)
      (if managedText (maybe-set-text-cursor gameobj))
    )
  )
)

(define (onObjUnselected)
  (unsetFocused)
  ;(format #t "on object unselected\n")
)

(define (enforceLayouts)
  ;; todo remove - no items in this layout, should require this 
  (enforce-layout (gameobj-id (lsobj-name "(banner_title_background_right" )))
  (enforce-layout (gameobj-id (lsobj-name "(banner_title_background_left" )))
  (enforce-layout (gameobj-id (lsobj-name "(test_panel")))
)
(enforceLayouts)

(define (onKey key scancode action mods)
  ;(format #t "action is: ~a\n" action)
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
    "< no data source >"
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
(define (update-binding attrpair getDataForAttr) 
  (define dataValue (getDataForAttr (cadr attrpair)))
  (define bindingIndex (caddr attrpair))
  ;(format #t "binding index: ~a ~a\n" bindingIndex (number? bindingIndex))
  (if (and bindingIndex (list? dataValue) (< bindingIndex (length dataValue)))
    (set! dataValue (list-ref dataValue bindingIndex))
  )
  (if (number? dataValue)
    (set! dataValue (number->string dataValue))
  )
  ;(format #t "data value: ~a\n" dataValue)
  (if (string? dataValue)
    (begin
      (gameobj-setattr! (car attrpair) 
        (list (list "value" dataValue))
      )
    )
    (format #t "warning not a string: ~a ~a ~a\n" attrpair dataValue bindingIndex)
  )
  attrpair
)

(define (update-toggle-binding attrpair getDataForAttr)
  (define toggleEnableText (getDataForAttr (cadr attrpair)))
  (define gameobj (car attrpair))
  (define bindingOn (assoc "details-binding-on" (gameobj-attr gameobj)))
  (define enableValueStr (if bindingOn (cadr bindingOn) "enabled"))
  (define enableValue (equal? enableValueStr toggleEnableText))
  ;(format #t "enable value str is: ~a for name ~a\n" enableValueStr (gameobj-name gameobj))
  ;(format #t "toggle enable text: ~a\n" toggleEnableText)
  ;(format #t "update toggle binding: ~a with value ~a (~a)\n" attrpair enableValue toggleEnableText)
  (gameobj-setattr! gameobj
    (list 
      (list "state" (if enableValue "on" "off"))
      (list "tint"  (if enableValue (list 0 0 1 1) (list 1 1 1 1)))
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


;;;;;;;;;;;;;;;

(define (getSlidePercentage id)
  (define gameobj (gameobj-by-id id))
  (define gameobjAttr (gameobj-attr gameobj))
  (define slideAmount (assoc "slideamount" gameobjAttr))
  (if slideAmount (list gameobj (cadr slideAmount) gameobjAttr) #f)
)

(define (onMessage key value)
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