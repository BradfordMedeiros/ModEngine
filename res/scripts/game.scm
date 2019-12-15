

(define (onMouse button action mods) (+ 0 0))
(define (onKeyChar codepoint)
  (display "codepoint is: ")
  (display codepoint)
  (display "\n")
)


(define selectionModeEnabled #f)
(define (onKey key scancode action mods)
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


(define (onCollide)
  (display "on collide placeholder")
)