

(define numFrames 0)
(define (incr) (set! numFrames (modulo (+ numFrames 1) 60)))

(define (onFrame)
	(incr)
	(if (= numFrames 0)
		(display "click\n")
	)
)