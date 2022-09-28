; TODO -> no reason to draw this every frame 

(define textureId #f)
(define texturename "editor-console")

(define (updateTexture)
	(gameobj-setattr! (lsobj-name "(console") 
  	(list
  		(list "texture" texturename)
  	)
  )
)

(set! textureId (create-texture texturename 2000 1000))
(updateTexture)
(enforce-layout (gameobj-id mainobj))

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

(define fontSize 8) ; 1000 means letter covers whole screen
(define letterSize (/ fontSize 1000))
(define margin (* 2 letterSize))
(define marginLeft margin)
(define marginTop margin)
(define paddingLeft (+ -1 marginLeft))
(define offsetY (- 1 letterSize marginTop))

(define (drawFrame numLines fontSize )
  (define currentHeight 0)
  (define spacing (* 4 letterSize))
  (draw-text-ndi "ModTerminal" paddingLeft (+ offsetY currentHeight) fontSize #f textureId)
  (draw-text-ndi "--------------------------------------" paddingLeft (+ offsetY currentHeight) fontSize #f textureId)
  (do ((row (- numLines 1) (- row 1))) ((< row 0))
    (let* ((historyLength (vector-length lineHistory)) (lineIndex (- historyLength row 1)))
      (if (and (< lineIndex historyLength) (>= lineIndex 0))
        (begin 
          (set! currentHeight (- currentHeight spacing))
          (draw-text-ndi (vector-ref lineHistory lineIndex) paddingLeft (+ offsetY currentHeight) fontSize #f textureId)
        )
      )
    )
  )
  (set! currentHeight (- currentHeight spacing))
  (draw-text-ndi "//////////////////////////////" paddingLeft (+ offsetY currentHeight) 2 #f textureId)
  (set! currentHeight (- currentHeight spacing))
  (draw-text-ndi (currentLineWithCursor currentLineBuffer cursorFromEnd) paddingLeft (+ offsetY currentHeight) fontSize #f textureId)
  (set! currentHeight (- currentHeight spacing))
  (draw-text-ndi "//////////////////////////////" paddingLeft (+ offsetY currentHeight) 2 #f textureId) 
)

(define buffer-size 85)
(define (onFrame)
  (if showConsole
    (begin
      (clear-texture textureId (list 0 0 0 0.8) )
      (drawFrame buffer-size fontSize)
    )
  )
  (maybe-move-panel)
)

(define expandedPosition (list 0 0 0))
(define foldedPosition (list 0 2 0))
(define maxMoveRatePerSecond 8)

(define (calcNextPosition currPos targetPos rate)
  (define currXPos (car currPos))
  (define currYPos (cadr currPos))
  (define currZPos 0)
  (define oldZPos (caddr currPos))
  (define targetXPos (car targetPos))
  (define targetYPos (cadr targetPos))
  (define targetZPos (caddr targetPos))
  (define xDiff (- currXPos targetXPos))
  (define yDiff (- currYPos targetYPos))
  (define zDiff (- currZPos targetZPos))
  (define distance (sqrt (+ (* xDiff xDiff) (* yDiff yDiff) (* zDiff zDiff))))
  (define projCurrPos (list (car currPos) (cadr currPos) 0))
  (if (< distance 0.1)
    (list (car targetPos) (cadr targetPos) oldZPos)
    (let ((newPosition (move-relative projCurrPos (orientation-from-pos projCurrPos targetPos) (* maxMoveRatePerSecond (time-elapsed)))))
      (list (car newPosition) (cadr newPosition) oldZPos) ; preserve z pos 
    )
  )
)
(define (maybe-move-panel)
  (define currentPosition (gameobj-pos mainobj))
  (if showConsole
    (gameobj-setpos! mainobj (calcNextPosition currentPosition expandedPosition maxMoveRatePerSecond))
    (gameobj-setpos! mainobj (calcNextPosition currentPosition foldedPosition maxMoveRatePerSecond))
  )
)
  
(define showConsole #f)

(define (tryUnlock)
  (define lockSuccess (lock "input"))
  (if lockSuccess
    (set! showConsole #t)
  )
)

(define (onKey key scancode action mods)
  (if (and (= key 96) (= action 1))
    (begin
      (if (not showConsole)
        (tryUnlock)
        (begin
          (unlock "input")
          (set! showConsole #f)
        )
        
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
)



