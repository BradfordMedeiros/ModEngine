(define allowed-messages '(
    "diffuse_on" 
    "diffuse_off"
    "paint_on"
    "paint_off"
    "bloom_on"
    "bloom_off"
    "highlight_on"
    "highlight_off"
  )
)

(define (makecamera) 
  (display "make camera placeholder\n")
)
(define (makelight)
  (display "make light placeholder\n")
)

(define fnMessages (list
  (list "makecamera" makecamera)
  (list "makelight"  makelight)
))

(define (setdrawopacity opacity) (set-fstate "opacity" opacity))
(define fnFloatMessages (list
  (list "drawopacity" setdrawopacity)
))


(define (onMessage message)
  (define fnToCall (assoc-ref fnMessages message))
  (if fnToCall
    ((car fnToCall))
    (if (member message allowed-messages)
      (set-state message)
    )
  )
)


(define (onFloatMessage message val)
  (define fnToCall (assoc-ref fnFloatMessages message))
  (if fnToCall
    ((car fnToCall) val)
  )
)

