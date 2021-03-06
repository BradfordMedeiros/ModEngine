(define attributeList '())
(define currGameobj #f)

(define (parseVecString vecString)
  (define stringList (filter (lambda (x) (> (string-length x) 0)) (string-split vecString #\ )))
  (define numList (map string->number stringList))
  (if (equal? (length numList) 3)
    numList
    #f
  )
)

(define (attrValueTyped attrValue originalAttrValue)
  (cond 
    ((list? originalAttrValue) (parseVecString attrValue))
    ((number? originalAttrValue) (string->number attrValue))
    ((string? originalAttrValue) attrValue)
    (#t #f)
  )
)

(define (submitAttributes attributeIndex newValue)
  (define attributeToUpdate (list-ref attributeList attributeIndex))
  (define attrName (car attributeToUpdate))
  (define attrValue (cadr attributeToUpdate))
  (define newPairValue (attrValueTyped newValue attrValue))
  (define newPair (list attrName newPairValue))
  (if newPairValue
    (begin
      (display "new pair is: ")
      (display newPair)
      (display "\n")
      (gameobj-setattr! currGameobj (list newPair))
      (set! attributeList (gameobj-attr currGameobj))
    )
  )
)

(define selectedIndex 0)
(define editingMode #f)
(define bufferValue "")

(define (upArrow)
  (set! selectedIndex (max (- selectedIndex 1) 0))
)
(define (downArrow)
  (set! selectedIndex (min (+ selectedIndex 1) (- (length attributeList) 1)))
)
(define (select)
  (if (equal? editingMode #t)
    (begin
      (submitAttributes selectedIndex bufferValue)
      (set! bufferValue "")
    )
  )
  (set! editingMode (not editingMode))
)
(define (backspace)
  (set! bufferValue (substring bufferValue 0 (max 0 (- (string-length bufferValue) 1))))
)

(define (maybe-highlight value index iskey)
  (if (equal? iskey #t)
    (if (equal? selectedIndex index)
      (string-append "> " value)
      value
    )
    (if (and (equal? selectedIndex index) (equal? editingMode #t)) 
      bufferValue
      value
    )
  )
)

(define (convert-type value)
  (cond 
    ((list? value) (string-append "(" (string-join (map number->string value) " ") ")"))
    ((number? value) (number->string value))
    (#t value)
  )
)

(define (drawAttribute typeValue currentHeight index)
  (define value (list (car typeValue) (convert-type (cadr typeValue))))
  (draw-text 
    (string-append 
      (string-pad-right (maybe-highlight (car value) index #t) 30) 
      "  " 
      (string-pad-right (maybe-highlight (cadr value) index #f) 30)
    ) 
    10 
    currentHeight 
    4
  )
)

(define (drawEditor currentHeight index)
  (if (< index (length attributeList))
    (begin
      (drawAttribute (list-ref attributeList index) currentHeight index)
      (drawEditor (+ currentHeight 10) (+ index 1))
    )
  )
)

(define (drawFrame offsetY)
  (define currentHeight offsetY)
  (draw-text "ModAttributes" 10 currentHeight 4)
  (set! currentHeight (+ currentHeight 10))
  (draw-text "//////////////////////////////" 10 currentHeight 2)
  (set! currentHeight (+ currentHeight 10))
  (drawEditor  currentHeight 0)
)

(define (onFrame)
  (if showConsole
    (drawFrame 400)
  )
)

(define showConsole #f)
(define (onKey key scancode action mods)
  (if (and (= key 96) (= action 1))
    (set! showConsole (not showConsole))
  )
  (if showConsole
    (if (and (= action 1) (not (= key 96)))
      (cond
        ((= key 257) (select)) 
        ((= key 265) (upArrow))    
        ((= key 264) (downArrow))  
        ((= key 259) (backspace)) 
      )
    )
  )
)

(define (onKeyChar key)
  (if (and showConsole (not (= key 96)))
    (set! bufferValue (string-append bufferValue (string (integer->char key))))
  )
)

(define (onObjSelected obj color)
  (define objattr (gameobj-attr obj))
  (display objattr)
  (set! currGameobj obj)
  (set! attributeList objattr)
)