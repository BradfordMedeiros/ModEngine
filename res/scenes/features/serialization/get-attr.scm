
(define (getType value) 
	(cond
		((string? value) "string")
		((number? value) "number")
		((list? value)
			(cond
				((equal? (length value) 3) "vec3")
				((equal? (length value) 4) "vec4")
				(#t "unknown vec")
			)

		)
		(#t "unknown")		
	)
)

(define (printPretty value)
	(format #t "(~a,~a[~a])\n" (car value) (cadr value) (getType (cadr value)))
)

(define (gettarget)
	(define scriptTarget (assoc "script-target" (gameobj-attr mainobj)))
	(if scriptTarget
		(lsobj-name (cadr scriptTarget))
		mainobj
	)
)


(define (onKey key scancode action mods)
	(if (and (equal? action 1) (equal? key 257))
		(map printPretty (gameobj-attr (gettarget)))
	)
	(format #t "\n")
)