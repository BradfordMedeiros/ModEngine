
(display "SYSTEM SCRIPT: MACROS INITIALIZED\n")

(define (getprint value) 
  (lambda () 
    (display (string-append "machine test: print test " value "\n"))
  )
)

;;;;;; CONVENIENCE SYNTAX FOR MACHINE ;;;;;;;;;;;;;;;;;;;;
(define old-machine machine)
(define-syntax machine
  (syntax-rules ()
    ((_ val body ...)
      (if (list? val)
        (old-machine val body ...)
        (old-machine (list val body ...))
        
      )
    )
    ((_ body ... ) (old-machine (list) body ...))
  )
)

;;;;;; CONVENIENCE SYNTAX FOR STATE ;;;;;;;;;;;;;;;;;;;;
(define old-state state)
(define-syntax state
  (syntax-rules ()
    ((_ name val body ...)
      (if (list? val)
        (old-state name val body ...)
        (old-state name (list val body ...))
      )
    )
    ((_ name body ... ) (old-state name (list) body ...))
  )
)

;;;; How to wrap body functions into an closure wrapper?
(define old-create-track create-track)
(define-syntax create-track
  (syntax-rules ()
    ((_ name val body ...)
      (if (list? val)
        (old-create-track name val body ...)
        (old-create-track name (list val body ...))
      )
    )
    ((_ name body ... ) (old-create-track name (list) body ...))
  )
)

(define defaultState 
  (state 
    "door-closed"
    (create-track "default" 
      (list 
        (getprint "100")
        (getprint "101")
        (getprint "102")
      )
    )
  )
)


(machine (list defaultState defaultState defaultState))


(machine 
  (state "runnign"
    ;(state-on-event "gunshot"
    ;  (play-animation "duck")
    ;)
    (create-track "default"
      (getprint "100")
      (getprint "101")
      (getprint "102")
      (display "hello world\n")
    )
    ;(state-on-exit
    ;  (stop-sound)
    ;)
  )
  (state "door-closed" 
    (create-track
      "default"
      (getprint "100")
      (getprint "101")
      (getprint "102")
    )
  )
)

;(play-machine main)


;(machine asdfasdf\52345)