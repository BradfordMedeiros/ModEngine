(define (playrecording)
  (play-recording (gameobj-id mainobj) "./res/scenes/features/recorder/camerasweep.rec")
)

(define (onKeyChar key)
  (cond 
    ((= key 46) (playrecording))     ; .
  )
)
