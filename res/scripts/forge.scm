
(define (upArrow)
  (set! selectedIndex (max (- selectedIndex 1) 0))
)
(define (downArrow)
  (set! selectedIndex (min (+ selectedIndex 1) (- (length menu) 1)))
)

(define displayObjName "debugobject")
(define num 0)
(define (getNextName)
  (set! num (+ num 1))
  (let ((nextName (string-append displayObjName (number->string num))))
    (if (not (equal? (lsobj-name nextName) #f))
      (getNextName)
      nextName
    )
  )
)
(define (select)
  (mk-obj (getNextName) (list-ref menu selectedIndex) '(0 0 0))
)

(define selectedIndex 0)
(define menu (ls-models))

(define (maybe-highlight text index)
  (if (equal? selectedIndex index)
    (string-append "> " text)
    text
  )
)
(define (drawList menu currentHeight index)
  (draw-text (maybe-highlight (car menu) index) 10 currentHeight 4)
  (if (> (length (cdr menu)) 0) 
    (drawList (cdr menu) (+ currentHeight 10) (+ index 1))
  )
)

(define (drawFrame numLines fontSize offsetY)
  (define currentHeight 0)
  (draw-text "ModForge" 10 (+ offsetY currentHeight) fontSize)
  (set! currentHeight (+ currentHeight 10))
  (draw-text "//////////////////////////////" 10 (+ offsetY currentHeight) 2)
  (drawList menu (+ offsetY (+ currentHeight 10)) 0)
  (draw-text "//////////////////////////////" 10 (+ offsetY currentHeight) 2)
)


(define buffer-size 20)
(define (onFrame)
  (if showConsole
    (drawFrame buffer-size 4 100)
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
      )
    )
  )
)



