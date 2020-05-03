
; TODO - this should be something that 
; - selects between objects;
; - displays the attributes
; - allows text input into it  


(define attributeList
  '(("position" "0 0 0")
    ("mesh" "./res/models/box/box.obj")
    ("enabled" "true")
  )
)

(define (drawAttribute value currentHeight)
  (draw-text 
    (string-append 
      (string-pad-right (car value) 15) 
      "  " 
      (string-pad-right (cadr value) 15)
    ) 
    10 
    currentHeight 
    4
  )
)

(define (drawEditor currentHeight index)
  (if (< index (length attributeList))
    (begin
      (drawAttribute (list-ref attributeList index) currentHeight)
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
  (set! currentHeight (+ currentHeight 10))
  (draw-text "//////////////////////////////" 10 currentHeight 2)
)


(define (onFrame)
  (if showConsole
    (drawFrame 100)
  )
)

(define (upArrow)
  (display "up placeholder\n")
)
(define (downArrow)
  (display "down placeholder\n")
)
(define (select)
  (display "select placeholder\n")
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



