

(define (onMouse button action mods) (+ 0 0))
(define (onMouseMove xPos yPos) (+ 0 0))
(define (onKeyChar codepoint) (+ 0 0))

(define (playanimation)
  (let* ((sentinel (lsobj-name "sentinel")) (animationname (car (gameobj-animations sentinel))))
    (gameobj-playanimation sentinel animationname)
  )
)

(define selectionModeEnabled #f)
(define (onKey key scancode action mods)
  (if (and (= key 75) (= action 1)) (playanimation))
  (if (and (= key 80) (= action 1))
    (begin 
    	(display "toggled selection mode")(display "\n")
		(set! selectionModeEnabled (not selectionModeEnabled))
  		(setSelectionMode selectionModeEnabled)
  	)
  )
)

(define (onObjSelected selectedIndex) 
  (+ 0 0)
)

(define (onFrame)
  (+ 0 0)
)


(define (onCollideEnter obj1 obj2)
  (display (string-append "collision enter " (number->string obj1) "-" (number->string obj2) "\n"))
)

(define (onCollideExit obj1 obj2)
  (display (string-append "collision exit" (number->string obj1) "-" (number->string obj2) "\n"))
)

(define (onCameraSystemChange type) #t)