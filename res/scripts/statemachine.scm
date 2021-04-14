
(define somemachine (machine
  (list
    (state "statemachine"
      (list
        (create-track "doing_whatever"
          (list
            (lambda () (display "Just doing whatever\n"))
          )
        )
      )
    )
  )
))

(play-machine somemachine)
