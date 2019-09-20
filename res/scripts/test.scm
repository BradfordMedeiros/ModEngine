
(define numFrames 0)
(define (incr) (set! numFrames (modulo (+ numFrames 1) 600)))

(drawText "hello wow" 100 100 3)

(define (nextNumber)
	(set! currentNumber (random 100))
)

(define (onKeyPressed button action mods)
	(display (string-append "button: " (number->string button) "\n"))
	(display (string-append "action: " (number->string action) "\n"))
	(display (string-append "mods: " (number->string mods) "\n"))

	(if (and (= button 1) (= action 0)) (nextNumber))
)


(define currentNumber 0)

(define (onFrame)
	(incr)
	(if (= numFrames 0) (nextNumber))
	(drawText (string-append "Scripting api test: " (number->string currentNumber)) 400 20 4)
	(if (= numFrames 0)
		(display "wow")
	)
)

;;p

