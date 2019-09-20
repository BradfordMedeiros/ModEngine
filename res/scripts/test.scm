
(define numFrames 0)
(define (incr) (set! numFrames (modulo (+ numFrames 1) 600)))


(define (nextNumber)
	(set! currentNumber (random 100))
)

(define (onMouse button action mods)
	(if (and (= button 1) (= action 0)) (nextNumber))
)

(define textBuffer "")
(define textToWrite "enter text here")

(define (onKey key scancode action mods)
	(if (= action 0) 
		(begin 
			(set! textToWrite textBuffer)
			(if (= key 257)
				(begin
					(display (string-append "flushing: " textBuffer "\n"))
					(set! textToWrite textBuffer)
					(set! textBuffer "")
				)
				(set! textBuffer (string-append textBuffer (string (integer->char key))))
			)
		)		
	)
)

(define (onObjSelected selectedIndex) 
	(display (string-append "object selected: " (number->string selectedIndex) "\n" ))
)

(define currentNumber 0)
(define (onFrame)
	(incr)
	(if (= numFrames 0) (nextNumber))
	(drawText (string-append "Scripting api test: " (number->string currentNumber)) 400 20 4)
	(drawText (string-append textToWrite) 400 30 2)
	(if (= numFrames 0)
		(display "wow")
	)
)

