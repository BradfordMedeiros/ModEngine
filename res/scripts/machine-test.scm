
(display (string-append "machine test: hello" "\n"))

(define (getprint value) 
  (lambda () 
    (display (string-append "machine test: print test " value "\n"))
  )
)

(define printtest1 (getprint "1"))
(define printtest2 (getprint "2"))
(define printtest3 (getprint "3"))

(define main (machine
  (list    
    (state 
      "door-open"
      (list 
        (create-track "default" 
          (list 
            (getprint "000")
            (getprint "001")
            (getprint "002")
            (lambda () (set-machine main "door-closed"))
          )
        )
      )
    )
    (state 
      "door-closed"
      (list 
        (create-track "default" 
          (list 
            (getprint "100")
            (getprint "101")
            (getprint "102")
          )
        )
      )
    )
  )
))

(play-machine main)
; set-machine machine value

;(play-track defaulttrack)