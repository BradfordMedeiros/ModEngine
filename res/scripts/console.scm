(use-modules (ice-9 eval-string))

(define (onMouse button action mods) (display ""))
(define (onObjSelected selectedIndex) (display ""))

(define lineHistory #())
(define (logToHistory lineText)
  (cond 
    ((equal? lineText ";clear")  (set! lineHistory #()))
    ((equal? lineText ";quit") (exit))
    (#t (begin 
      (set! lineHistory (list->vector (reverse (cons lineText (reverse (vector->list lineHistory))))))
      (display (string-append "evaling: " lineText "\n"))
      (eval-string lineText)
    ))
  )
)

(define currentLineBuffer "")
(define (appendToBuffer value)
  (set! currentLineBuffer (string-append currentLineBuffer (string-downcase value)))
)
(define (submitBuffer)
  (logToHistory currentLineBuffer)
  (set! currentLineBuffer "")
)
(define (backspaceBuffer)
  (set! currentLineBuffer (substring currentLineBuffer 0 (max 0 (- (string-length currentLineBuffer) 1))))
)


(define (drawFrame numLines fontSize offsetY)
  (define currentHeight 0)
  (draw-text "ModTerminal" 10 (+ offsetY currentHeight) fontSize)
  (set! currentHeight (+ currentHeight 10))
  (draw-text "--------------------------------------" 10 (+ offsetY currentHeight) 1)
  (do ((row (- numLines 1) (- row 1))) ((< row 0))
    (let* ((historyLength (vector-length lineHistory)) (lineIndex (- historyLength row)))
      (if (and (< lineIndex historyLength) (>= lineIndex 0))
        (begin 
          (set! currentHeight (+ currentHeight 10))
          (draw-text (vector-ref lineHistory lineIndex) 10 (+ offsetY currentHeight) fontSize)
        )
      )
    )
  )
  (set! currentHeight (+ currentHeight 10))
  (draw-text "//////////////////////////////" 10 (+ offsetY currentHeight) 2)
  (set! currentHeight (+ currentHeight 10))
  (draw-text currentLineBuffer 10 (+ offsetY currentHeight) fontSize)
  (set! currentHeight (+ currentHeight 10))
  (draw-text "//////////////////////////////" 10 (+ offsetY currentHeight) 2)
)


(define (onFrame)
  (if showConsole
    (drawFrame 10 4 100)
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
        ((= key 257) (submitBuffer)) 
        ((= key 259) (backspaceBuffer))
        (#t (appendToBuffer (string (integer->char key))))
      )
    )
  )
)

