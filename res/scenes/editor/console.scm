;(define (onFrame)
;	(format #t "on frame console.scm!\n")
;)

(define textureId #f)
(define texturename "editor-console")

(define (updateTexture)
	(gameobj-setattr! (lsobj-name "(console") 
  	(list
  		(list "texture" texturename)
  	)
  )
)


(set! textureId (create-texture texturename 1000 1000))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(use-modules (ice-9 eval-string))
(use-modules (ice-9 format))

(define activeLineIndex 0)
(define lineHistory #())
(define cursorFromEnd 0)
(define (appendHistory lineText)
  (set! lineHistory (list->vector (reverse (cons lineText (reverse (vector->list lineHistory))))))
)

(define (catchAsString thunk)
  (catch #t thunk
    (lambda (key . parameters)
      (format (current-error-port) "Uncaught throw to '~a: ~a\n" key parameters)
      "<< ERROR >>"
    )
  )
)


(define (executeCommand lineText)
  (cond 
    ((equal? lineText ";clear")  (set! lineHistory #()))
    ((equal? lineText ";quit") (exit))
    (#t (begin 
      (appendHistory lineText)
      (let ((result (catchAsString (lambda () (eval-string lineText)))))
        (appendHistory (format #f "; >result: ~y" result))
      )
    ))
  )
)

(define (splitString line cursorFromEnd)
  (list (substring line 0 (- (string-length line) cursorFromEnd)) (substring line (- (string-length line) cursorFromEnd) (string-length line))) 
)

(define currentLineBuffer "")
(define (appendToBuffer value)
  (let ((splittedString (splitString currentLineBuffer cursorFromEnd)))
    (set! currentLineBuffer (string-append (car splittedString) value (cadr splittedString)))
  )
)
(define (submitBuffer)
  (executeCommand currentLineBuffer)
  (set! currentLineBuffer "")
  (set! activeLineIndex (vector-length lineHistory))
  (set! cursorFromEnd 0)
)
(define (backspaceBuffer)
  (if (not (equal? cursorFromEnd (string-length currentLineBuffer)))
    (let* ((splittedString (splitString currentLineBuffer cursorFromEnd)) (firstString (car splittedString)) (secString (cadr splittedString)))
      (set! currentLineBuffer (string-append (substring firstString 0 (- (string-length firstString) 1)) secString))
    )
  )
)
(define (upArrow)
  (if (not (= activeLineIndex 0))
    (begin
      (set! activeLineIndex (- activeLineIndex 1))
      (set! currentLineBuffer (vector-ref lineHistory activeLineIndex))
      (set! cursorFromEnd 0)
    )
  )
)
(define (downArrow)
  (if (< activeLineIndex (- (vector-length lineHistory) 1))
    (begin
      (set! activeLineIndex (+ activeLineIndex 1))
      (set! currentLineBuffer (vector-ref lineHistory activeLineIndex))
      (set! cursorFromEnd 0)
    )
  )
)
(define (leftArrow)  (if (< cursorFromEnd (string-length currentLineBuffer)) (set! cursorFromEnd (+ cursorFromEnd 1))))
(define (rightArrow) (if (> cursorFromEnd 0 )                                (set! cursorFromEnd (- cursorFromEnd 1))))


(define (currentLineWithCursor line cursorPos)
  (let ((newString (splitString line cursorPos)))
    (string-append (car newString) "|" (cadr newString))
  )
)
(define (drawFrame numLines fontSize offsetY)
  (define currentHeight 0)
  (draw-text "ModTerminal" 10 (+ offsetY currentHeight) fontSize #f textureId)
  (set! currentHeight (+ currentHeight 10))
  (draw-text "--------------------------------------" 10 (+ offsetY currentHeight) 1 #f textureId)
  (do ((row (- numLines 1) (- row 1))) ((< row 0))
    (let* ((historyLength (vector-length lineHistory)) (lineIndex (- historyLength row 1)))
      (if (and (< lineIndex historyLength) (>= lineIndex 0))
        (begin 
          (set! currentHeight (+ currentHeight 10))
          (draw-text (vector-ref lineHistory lineIndex) 10 (+ offsetY currentHeight) fontSize #f textureId)
        )
      )
    )
  )
  (set! currentHeight (+ currentHeight 10))
  (draw-text "//////////////////////////////" 10 (+ offsetY currentHeight) 2 #f textureId)
  (set! currentHeight (+ currentHeight 10))
  (draw-text (currentLineWithCursor currentLineBuffer cursorFromEnd) 10 (+ offsetY currentHeight) fontSize #f textureId)
  (set! currentHeight (+ currentHeight 10))
  (draw-text "//////////////////////////////" 10 (+ offsetY currentHeight) 2 #f textureId) 
)


(define buffer-size 20)
(define (onFrame)
	(clear-texture textureId (list 0.1 0.1 0.1 0.8))
  (if showConsole
    (drawFrame buffer-size 4 100)
  )
)

(define showConsole #f)
(define (onKey key scancode action mods)
  (if (and (= key 96) (= action 1))
    (begin
      (set! showConsole (not showConsole))
      (if showConsole
        (lock "input")
        (unlock "input")
      )
    )
  )
  (if showConsole
    (if (and (= action 1) (not (= key 96)))
      (cond
        ((= key 257) (submitBuffer)) 
        ((= key 259) (backspaceBuffer))
        ((= key 265) (upArrow))    
        ((= key 264) (downArrow))   
        ((= key 263) (leftArrow))    
        ((= key 262) (rightArrow))  
        (#t (+ 0 0))
      )
    )
  )
)

(define (onKeyChar key)
  (if (and showConsole (not (= key 96)))
    (appendToBuffer (string (integer->char key)))
  )
 	(if (equal? key 47) 
		(updateTexture)
	)   
)



